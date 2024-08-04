// Copyright Samuel Reitich. All rights reserved.


#include "CrashGameplayAbilityBase.h"

#include "Abilities/Tasks/AbilityTask.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/GameplayEffects/CrashGameplayEffectContext.h"
#include "AbilitySystemLog.h"
#include "Characters/PawnCameraManager.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayAbility"

// Helper for functions that require an instantiated ability.
#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																					\
{																																							\
	if (!ensure(IsInstantiated()))																															\
	{																																						\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());		\
		return ReturnValue;																																	\
	}																																						\
}

UCrashGameplayAbilityBase::UCrashGameplayAbilityBase(const FObjectInitializer& ObjectInitializer)
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	
	ActivationMethod = EAbilityActivationMethod::OnInputTriggered;
	ActivationGroup = EAbilityActivationGroup::Independent;

	bIsUserFacingAbility = false;
	AbilityIcon = nullptr;
}

void UCrashGameplayAbilityBase::TryActivatePassiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
	if (ActivationMethod != EAbilityActivationMethod::Passive)
	{
		return;
	}

	// Do not predict the activation of passive abilities.
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Ensure we have valid actor info, the ability is not already active, and we are not predicting the activation.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting)
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		const AActor* Avatar = ActorInfo->AvatarActor.Get();

		// Don't activate the passive ability if the avatar has been torn off or is about to die.
		if (ASC && Avatar && !Avatar->GetTearOff() && (Avatar->GetLifeSpan() <= 0.0f))
		{
			// Whether this ability should be activated from the client and/or server.
			const bool bExecutesOnClient = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			const bool bExecutesOnServer = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			// Whether we can activate this ability from this machine, depending on our net mode.
			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bExecutesOnClient;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bExecutesOnServer;

			// Activate the ability on this machine, if we can.
			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

bool UCrashGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Check if this ability is explicitly disabled.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_Ability_Behavior_Disabled))
	{
		return false;
	}

	// Check if this ability's activation group is currently blocked on its ASC.
	if (GetCrashAbilitySystemComponentFromActorInfo() && GetCrashAbilitySystemComponentFromActorInfo()->IsActivationGroupBlocked(GetActivationGroup()))
	{
		return false;
	}

	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UCrashGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponentFromActorInfo();
	check(CrashASC);

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Create an effect context with which to apply this ability's effects.
	const FGameplayEffectContextHandle EffectContextHandle = CrashASC->MakeEffectContext();

	// Apply every ongoing effect that will NOT be automatically removed when this ability ends.
	for (auto GE : OngoingEffectsToApplyOnStart)
	{
		if (!GE.Get())
		{
			continue;
		}

		// Make an outgoing spec handle for the effect.
		FGameplayEffectSpecHandle EffectSpecHandle = CrashASC->MakeOutgoingSpec(GE, 1, EffectContextHandle);
		if (!EffectSpecHandle.IsValid())
		{
			continue;
		}

		// Apply the effect.
		FActiveGameplayEffectHandle ActiveEffectHandle = CrashASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		if (!ActiveEffectHandle.WasSuccessfullyApplied())
		{
			ABILITY_LOG(Warning, TEXT("Ability [%s] failed to apply ongoing effect [%s]."), *GetName(), *GetNameSafe(GE));
		}
	}

	/* Abilities must be instantiated to store handles to automatically remove effects when the ability ends. If
	 * this ability is not instantiated, it cannot apply automatically-removed ongoing effects. */ 
	if (!IsInstantiated() && OngoingEffectsToApplyOnStartAndRemoveOnEnd.Num())
	{
		ABILITY_LOG(Warning, TEXT("Ability [%s] tried to apply temporary ongoing abilities, but is not instantiated. Abilities must be instantiated to apply automatically-removed ongoing effects."), *GetName());
	}
	else
	{
		// Apply every ongoing effect that WILL be automatically removed when this ability ends.
		for (auto GE : OngoingEffectsToApplyOnStartAndRemoveOnEnd)
		{
			if (!GE.Get())
			{
				continue;
			}

			// Make an outgoing spec handle for the effect.
			FGameplayEffectSpecHandle EffectSpecHandle = CrashASC->MakeOutgoingSpec(GE, 1, EffectContextHandle);
			if (!EffectSpecHandle.IsValid())
			{
				continue;
			}

			// Apply the effect.
			FActiveGameplayEffectHandle ActiveEffectHandle = CrashASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

			/* Save the effect's handle if it was successfully applied so it can automatically be removed when this
			 * ability ends. */
			if (ActiveEffectHandle.WasSuccessfullyApplied())
			{
				EffectsToRemoveOnEndHandles.Add(ActiveEffectHandle);
			}
			else
			{
				ABILITY_LOG(Warning, TEXT("Ability [%s] failed to apply ongoing effect [%s]."), *GetName(), *GetNameSafe(GE));
			}
		}
	}

	// Update this ability's activation group if it was successfully activated.
	CrashASC->HandleAbilityActivatedForActivationGroup(this);
}

void UCrashGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponentFromActorInfo();
	check(CrashASC);

	// Before ending this ability, remove all ongoing effects that are marked to be removed when the ability ends.
	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : EffectsToRemoveOnEndHandles)
	{
		if (ActiveEffectHandle.IsValid())
		{
			CrashASC->RemoveActiveGameplayEffect(ActiveEffectHandle);
		}
	}

	// Reset the array of effect handles to remove on ability end.
	EffectsToRemoveOnEndHandles.Reset();

	// Update this ability's activation group after it ends.
	CrashASC->HandleAbilityEndedForActivationGroup(this);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCrashGameplayAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (const UGameplayEffect* CooldownGE = GetCooldownGameplayEffect())
	{
		// Apply the cooldown manually so we can get a reference to the active effect.
		const FActiveGameplayEffectHandle CooldownEffectHandle = ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CooldownGE, GetAbilityLevel(Handle, ActorInfo));

		if (CooldownEffectHandle.WasSuccessfullyApplied())
		{
			// Send a standardized message that this ability's cooldown started.
			if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
			{
				FCrashAbilityMessage AbilityMessage = FCrashAbilityMessage();
				AbilityMessage.MessageType = CrashGameplayTags::TAG_Message_Ability_Cooldown_Started;
				AbilityMessage.AbilitySpecHandle = Handle;
				AbilityMessage.ActorInfo = *GetCrashActorInfo();
				const FActiveGameplayEffect* CooldownEffect = GetAbilitySystemComponentFromActorInfo_Checked()->GetActiveGameplayEffect(CooldownEffectHandle);
				AbilityMessage.OptionalMagnitude = CooldownEffect->GetDuration();

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(AbilityMessage.MessageType, AbilityMessage);

				// TODO: Broadcast the message to clients, since ApplyCooldown is only called on the server.
				if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(GetWorld())))
				{
					// GS->MulticastReliableAbilityMessageToClients(AbilityMessage);
				}
			}
		}
	}
}

void UCrashGameplayAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Trigger the blueprint-exposed version of this callback.
	K2_OnGiveAbility();

	// Try to activate passive abilities when they are given to an ASC.
	TryActivatePassiveAbility(ActorInfo, Spec);
}

void UCrashGameplayAbilityBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	// Trigger the blueprint-exposed version of this callback.
	K2_OnRemoveAbility();
}

void UCrashGameplayAbilityBase::OnNewAvatarSet()
{
	// Trigger the blueprint-exposed version of this callback.
	K2_OnNewAvatarSet();
}

void UCrashGameplayAbilityBase::AddKnockback(float Force, FVector Source, AActor* Target, AActor* Instigator)
{
	if (!ensure(Target))
	{
		return;
	}

	FRotator DirectionRot = UKismetMathLibrary::FindLookAtRotation(Source, Target->GetActorLocation());
	FVector Direction = DirectionRot.Vector();
	AddKnockback_Internal((Force * Direction), Target, Instigator);
}

void UCrashGameplayAbilityBase::AddKnockback_Internal(FVector Force, AActor* Target, AActor* Instigator)
{
	// If the target actor is a character, add the impulse to their movement component.
	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		if (UCharacterMovementComponent* TargetMovementComp = TargetChar->GetCharacterMovement())
		{
			TargetMovementComp->AddImpulse(Force);
		}
	}
	// If the target actor has a primitive root with physics enabled, add the impulse to their root.
	else if (UPrimitiveComponent* RootScene = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		if (RootScene->IsAnySimulatingPhysics())
		{
			RootScene->AddImpulse(Force);
		}
	}

	// TODO: Broadcast message to credit upcoming knockback kills, until the target touches the ground again.
}

void UCrashGameplayAbilityBase::SetCameraMode(TSubclassOf<UCrashCameraModeBase> CameraMode)
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(SetCameraMode, );

	// Set the overriding camera mode via the avatar's pawn camera manager component.
	if (UPawnCameraManager* PawnCameraManager = CurrentActorInfo ? UPawnCameraManager::FindPawnCameraManagerComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		PawnCameraManager->SetAbilityCameraMode(CameraMode, CurrentSpecHandle);
		ActiveCameraMode = CameraMode;
	}
}

void UCrashGameplayAbilityBase::ClearCameraMode()
{
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ClearCameraMode, );

	// Clear the overriding camera mode via the avatar's pawn camera manager component.
	if (UPawnCameraManager* PawnCameraManager = CurrentActorInfo ? UPawnCameraManager::FindPawnCameraManagerComponent(CurrentActorInfo->AvatarActor.Get()) : nullptr)
	{
		PawnCameraManager->ClearAbilityCameraMode(CurrentSpecHandle);
	}

	ActiveCameraMode = nullptr;
}

FGameplayEffectContextHandle UCrashGameplayAbilityBase::MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	/* Create an effect context of type CrashEffectContext. Our AbilitySystemGlobals class defines the correct effect
	 * context type. */
	FGameplayEffectContextHandle OutEffectContextHandle = Super::MakeEffectContext(Handle, ActorInfo);

	/* Ensure the new effect context is the correct type as defined in our AbilitySystemGlobals. */
	const FCrashGameplayEffectContext* CrashEffectContext = FCrashGameplayEffectContext::GetCrashContextFromHandle(OutEffectContextHandle);
	check(CrashEffectContext);

	return OutEffectContextHandle;
}

const FCrashGameplayAbilityActorInfo* UCrashGameplayAbilityBase::GetCrashActorInfo() const
{
	// The current actor info should always be of type FCrashGameplayAbilityActorInfo. This should never fail.
	return static_cast<const FCrashGameplayAbilityActorInfo*>(CurrentActorInfo);
}

FCrashGameplayAbilityActorInfo UCrashGameplayAbilityBase::K2_GetCrashActorInfo() const
{
	// We should always have actor info. But we still have to return something if we don't.
	if (!ensure(CurrentActorInfo))
	{
		return FCrashGameplayAbilityActorInfo();
	}

	return *static_cast<const FCrashGameplayAbilityActorInfo*>(CurrentActorInfo);
}

UCrashAbilitySystemComponent* UCrashGameplayAbilityBase::GetCrashAbilitySystemComponentFromActorInfo() const
{
	// Retrieve the typed ASC cached by our custom actor info.
	return (CurrentActorInfo ? GetCrashActorInfo()->GetCrashAbilitySystemComponent() : nullptr);
}

ACrashPlayerController* UCrashGameplayAbilityBase::GetCrashPlayerControllerFromActorInfo() const
{
	// Retrieve the actor info's typed PC.
	return GetCrashActorInfo()->GetCrashPlayerController();
}

AController* UCrashGameplayAbilityBase::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		// Try to get the owning actor's player controller.
		if (AController* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// If the owning actor's player controller isn't found, look for a controller anywhere in the owner chain.
		AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (AController* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (APawn* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ACrashPlayerState* UCrashGameplayAbilityBase::GetCrashPlayerStateFromActorInfo() const
{
	// Retrieve the actor info's typed PS.
	return GetCrashActorInfo()->GetCrashPlayerState();
}

#if WITH_EDITOR
EDataValidationResult UCrashGameplayAbilityBase::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (bReplicateInputDirectly)
	{
		Context.AddError(LOCTEXT("ReplicateInputDirectlyNotSupported", "This project does not support directly replicating ability input. Use a WaitInputPress or WaitInputRelease task instead."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE