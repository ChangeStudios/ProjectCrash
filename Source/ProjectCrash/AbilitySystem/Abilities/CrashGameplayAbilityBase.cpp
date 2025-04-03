// Copyright Samuel Reitich. All rights reserved.


#include "CrashGameplayAbilityBase.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilitySystem/AbilitySystemUtilitiesLibrary.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/GameplayEffects/CrashGameplayEffectContext.h"
#include "Blueprint/UserWidget.h"
#include "Characters/CrashCharacter.h"
#include "Characters/PawnCameraManager.h"
#include "Development/CrashDeveloperSettings.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayAbility"

/** Helper for functions that require an instantiated ability. */
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

	CooldownDuration = 0.0f;
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

bool UCrashGameplayAbilityBase::CheckCanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Explicitly disabled.
	if (AbilityTags.HasTagExact(CrashGameplayTags::TAG_Ability_Behavior_Disabled))
	{
		return false;
	}

	/* Activation group. We only check this when this ability is inactive, because we don't want an ability to think
	 * it's blocking itself. */
	if (!IsActive())
	{
		if (GetCrashAbilitySystemComponentFromActorInfo() && GetCrashAbilitySystemComponentFromActorInfo()->IsActivationGroupBlocked(GetActivationGroup()))
		{
			return false;
		}
	}

	// Valid avatar.
	AActor* const AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
	if (AvatarActor == nullptr || !ShouldActivateAbility(AvatarActor->GetLocalRole()))
	{
		return false;
	}

	// Make into a reference for simplicity.
	static FGameplayTagContainer DummyContainer;
	DummyContainer.Reset();
	FGameplayTagContainer& OutTags = OptionalRelevantTags ? *OptionalRelevantTags : DummyContainer;

	// Valid ASC.
	UAbilitySystemComponent* const AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return false;
	}

	// Valid spec.
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		return false;
	}

	// Input is inhibited (e.g. UI is pulled up).
	if (AbilitySystemComponent->GetUserAbilityActivationInhibited())
	{
		return false;
	}

	// Tag requirements.
	if (!DoesAbilitySatisfyTagRequirements(*AbilitySystemComponent, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// (Input ID check would be performed here, but this project doesn't use it)

	// Blueprint implementation.
	if (bHasBlueprintCanUse)
	{
		if (K2_CanActivateAbility(*ActorInfo, Handle, OutTags) == false)
		{
			return false;
		}
	}

	return true;
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
		// Add this ability's cooldown tags to the cooldown GE.
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGE->GetClass(), GetAbilityLevel());
		SpecHandle.Data.Get()->DynamicGrantedTags.AppendTags(CooldownTags);

		// Override the cooldown GE's duration if it uses a set-by-caller duration.
		if (ShouldSetCooldownDuration())
		{
			check(CooldownDuration > 0.0f);
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(CrashGameplayTags::TAG_GameplayEffects_SetByCaller_CooldownDuration, CooldownDuration);
		}

		// Apply the cooldown.
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		FActiveGameplayEffectHandle CooldownEffectHandle;

		// If our current prediction key is still valid, use it to predict our cooldown.
		if (ASC && ASC->ScopedPredictionKey.IsValidForMorePrediction())
		{
			CooldownEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
		}
		/* Create and confirm a new prediction window for applying the cooldown, if necessary (e.g. if we're applying it
		 * at the end of a latent ability, like the end of a dash, when the original prediction key has already been
		 * sent to the server). */
		else
		{
			FScopedPredictionWindow ScopedPrediction(ASC, true);
			CooldownEffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
			if (IsPredictingClient())
			{
				GetAbilitySystemComponentFromActorInfo()->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GameCustom1 /* Cooldown start. */, Handle, ActivationInfo.GetActivationPredictionKey(), ASC->ScopedPredictionKey);
			}
		}

		/* Broadcast relevant messages if the cooldown was successfully applied.
		 * NOTE: We intentionally predict the cooldown, but not the cooldown message. We always want the client to be
		 * up-to-date with the state of their ability, but we never want our messages to fall out of sync with the
		 * server. For example, predicting the cooldown lets us always accurately determine whether our charge-based
		 * ability's widget needs to enter a cooldown animation or if it can immediately be used again. Either way,
		 * by NOT predicting the message that TRIGGERS the animation, the cooldown timer itself will always be synced
		 * with the server. */
		if (CooldownEffectHandle.WasSuccessfullyApplied() && ASC->IsOwnerActorAuthoritative())
		{
			UCrashAbilitySystemComponent* CrashASC = Cast<UCrashAbilitySystemComponent>(ASC);
			check(CrashASC);

			// Send a standardized message that this ability's cooldown started.
			const float CooldownTime = CrashASC->GetActiveGameplayEffect(CooldownEffectHandle)->GetTimeRemaining(GetWorld()->GetTimeSeconds());
			CrashASC->BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started, GetCurrentAbilitySpecHandle(), CooldownTime, true);

			// Listen for the cooldown effect to be removed.
			CrashASC->OnAnyGameplayEffectRemovedDelegate().AddWeakLambda(this, [this, CooldownEffectHandle, CrashASC](const FActiveGameplayEffect& RemovedEffect)
			{
				if (RemovedEffect.Handle == CooldownEffectHandle)
				{
					CrashASC->BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended, GetCurrentAbilitySpecHandle(), 0.0f, true);
				}
			});
		}
	}
}

const FGameplayTagContainer* UCrashGameplayAbilityBase::GetCooldownTags() const
{
	FGameplayTagContainer* MutableTags = const_cast<FGameplayTagContainer*>(&CooldownTags_Internal);
	MutableTags->Reset(); // Tags are retrieved through the CDO, so they need to be reset each time.

	// Add cooldown GE's tags.
	if (const FGameplayTagContainer* ParentTags = Super::GetCooldownTags())
	{
		MutableTags->AppendTags(*ParentTags);
	}

	// Add this ability's cooldown tags.
	MutableTags->AppendTags(CooldownTags);

	return MutableTags;
}

bool UCrashGameplayAbilityBase::ShouldSetCooldownDuration() const
{
	// Abilities can only set their cooldown duration directly if their cooldown GE has a set-by-caller duration.
	if (IsValid(CooldownGameplayEffectClass))
	{
		if (const UGameplayEffect* CDO = CooldownGameplayEffectClass.GetDefaultObject())
		{
			return (CDO->DurationPolicy == EGameplayEffectDurationType::HasDuration) &&
				(CDO->DurationMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller);
		}
	}

	return false;
}

void UCrashGameplayAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	/* Register and initialize this ability's widgets for the owning player. We use the given actor info's player state
	 * because this ability's actor info may not be replicated to clients yet. */
	APlayerState* PS = Cast<APlayerState>(ActorInfo->OwnerActor);
	AController* Controller = PS ? PS->GetOwningController() : nullptr;
	if (Controller && Controller->IsLocalController())
	{
		if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
		{
			for (auto KVP : AbilityWidgets)
			{
				if (ensure(KVP.Key.IsValid() && IsValid(KVP.Value)))
				{
					// TODO: Check if this is getting called before this ability's actor info is set. GetCrashPlayerStateFromActorInfo will cause crashes if it is. 
					AbilityWidgetHandles.Add(ExtensionSubsystem->RegisterExtensionAsWidgetForContext(KVP.Key, GetCrashPlayerStateFromActorInfo(), KVP.Value, -1));
				}
			}
		}
	}

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

	// Remove this ability's widgets.
	if (UUIExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UUIExtensionSubsystem>())
	{
		for (FUIExtensionHandle& Extension : AbilityWidgetHandles)
		{
			if (Extension.IsValid())
			{
				ExtensionSubsystem->UnregisterExtension(Extension);
			}
		}
	}
}

void UCrashGameplayAbilityBase::OnNewAvatarSet()
{
	// Trigger the blueprint-exposed version of this callback.
	K2_OnNewAvatarSet();
}

void UCrashGameplayAbilityBase::AddKnockbackToTargetFromLocation(float Force, FVector Source, AActor* Target)
{
	UAbilitySystemUtilitiesLibrary::ApplyKnockbackToTargetFromLocation(Force, Source, Target, GetOwningActorFromActorInfo());
}

void UCrashGameplayAbilityBase::AddKnockbackToTargetInDirection(FVector Force, AActor* Target)
{
	UAbilitySystemUtilitiesLibrary::ApplyKnockbackToTargetInDirection(Force, Target, GetOwningActorFromActorInfo());
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
	return (CurrentActorInfo ? GetCrashActorInfo()->GetCrashAbilitySystemComponent() : nullptr);
}

ACrashCharacter* UCrashGameplayAbilityBase::GetCrashCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACrashCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

ACrashPlayerController* UCrashGameplayAbilityBase::GetCrashPlayerControllerFromActorInfo() const
{
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

FGameplayTargetDataFilterHandle UCrashGameplayAbilityBase::MakeCrashFilter(FCrashTargetDataFilter Filter)
{
	return UAbilitySystemUtilitiesLibrary::MakeCrashFilterHandle(Filter, GetAvatarActorFromActorInfo());
}

#if WITH_EDITOR
EDataValidationResult UCrashGameplayAbilityBase::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Disable support for bReplicateInputDirectly.
	if (bReplicateInputDirectly)
	{
		Context.AddError(LOCTEXT("ReplicateInputDirectlyNotSupported", "This project does not support directly replicating ability input. Use a WaitInputPress or WaitInputRelease task instead."));
		Result = EDataValidationResult::Invalid;
	}

	// Ensure we have a valid cooldown duration, if it will be used.
	if (ShouldSetCooldownDuration() && !(CooldownDuration > 0.0f))
	{
		Context.AddError(LOCTEXT("CooldownTooShort", "This ability's cooldown gameplay effect will be overridden by this ability, but its cooldown is too short. Either make the cooldown duration greater than 0.0s, or use a cooldown GE that does not use set-by-caller for its duration."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif // WITH_EDITOR

#if WITH_EDITOR
bool UCrashGameplayAbilityBase::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);

	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		// Cooldown duration should only be editable if it will actually be used.
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UCrashGameplayAbilityBase, CooldownDuration))
		{
			bIsEditable = ShouldSetCooldownDuration();
		}
	}

	return bIsEditable;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE