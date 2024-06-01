// Copyright Samuel Reitich 2024.


#include "CrashGameplayAbilityBase.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "NativeGameplayTags.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Effects/CrashGameplayEffectContext.h"
#include "Characters/ChallengerBase.h"
#include "Characters/Data/ChallengerSkinData.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameStates/CrashGameState_DEP.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerStates/CrashPlayerState.h"

UCrashGameplayAbilityBase::UCrashGameplayAbilityBase(const FObjectInitializer& ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationGroup = EAbilityActivationGroup::Independent;
	DisplayedAbilityName = "";
	DisplayedAbilityDescription = "";
	AbilityIcon = nullptr;

	if (InstancingPolicy != EGameplayAbilityInstancingPolicy::InstancedPerActor)
	{
		ABILITY_LOG(Warning, TEXT("Ability [%s] is not instanced-per-actor. Certain activation style features may not work properly."), *GetName());
	}
}

FGameplayTag UCrashGameplayAbilityBase::GetAbilityCueFromSkin(FGameplayTag DefaultCue)
{
	check(DefaultCue.IsValid());
	const FGameplayTag Parent = DefaultCue.RequestDirectParent();

	// Retrieve the current skin from this ability's owner.
	const ACrashPlayerState* CrashPS = Cast<ACrashPlayerState>(CurrentActorInfo->OwnerActor);
	const UChallengerSkinData* SkinData = CrashPS ? CrashPS->GetCurrentSkin() : nullptr;

	// Find a matching ability cue.
	if (SkinData)
	{
		for (FGameplayTag AbilityCue : SkinData->AbilityCues)
		{
			if (AbilityCue.RequestDirectParent().MatchesTag(Parent))
			{
				return AbilityCue;
			}
		}
	}

	// If a matching cue can't be found, return the default cue, which was given.
	return DefaultCue;
}

UCrashAbilitySystemComponent* UCrashGameplayAbilityBase::GetCrashAbilitySystemComponentFromActorInfo() const
{
	// Retrieve the ASC from the current actor info and cast it to a CrashAbilitySystemComponent.
	return CurrentActorInfo && CurrentActorInfo->AbilitySystemComponent.Get() ? Cast<UCrashAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get()) : nullptr;
}

AChallengerBase* UCrashGameplayAbilityBase::GetChallengerFromActorInfo() const
{
	// Retrieve the avatar from the current actor info and cast it to AChallengerBase.
	return CurrentActorInfo && GetAvatarActorFromActorInfo() ? Cast<AChallengerBase>(GetAvatarActorFromActorInfo()) : nullptr;
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

#if WITH_EDITOR

bool UCrashGameplayAbilityBase::CanEditChange(const FProperty* InProperty) const
{
	bool bIsMutable = Super::CanEditChange(InProperty);

	if (bIsMutable && InProperty != NULL)
	{
		const FName PropName = InProperty->GetFName();

		// Only display the ability icon property if this ability will appear in the UI.
		if (PropName == GET_MEMBER_NAME_CHECKED(UCrashGameplayAbilityBase, AbilityIcon))
		{
			bIsMutable = UserInterfaceTags.HasTag(CrashGameplayTags::TAG_Ability_Behavior_UI_Slotted_Generic.GetTag().RequestDirectParent());
		}
		/* Only display frontend ability properties if this ability will appear in the frontend (e.g. character
		 * selection screens). */
		else if (PropName == GET_MEMBER_NAME_CHECKED(UCrashGameplayAbilityBase, DisplayedAbilityName) ||
			PropName == GET_MEMBER_NAME_CHECKED(UCrashGameplayAbilityBase, DisplayedAbilityDescription))
		{
			bIsMutable = UserInterfaceTags.HasTag(CrashGameplayTags::TAG_Ability_Behavior_UI_Frontend_PrimaryWeapon.GetTag().RequestDirectParent());
		}
	}

	return bIsMutable;
}

#endif // #if WITH_EDITOR

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
				AbilityMessage.Ability = GetAbilityCDO();
				AbilityMessage.ActorInfo = GetActorInfo();
				const FActiveGameplayEffect* CooldownEffect = GetAbilitySystemComponentFromActorInfo_Checked()->GetActiveGameplayEffect(CooldownEffectHandle);
				AbilityMessage.OptionalMagnitude = CooldownEffect->GetDuration();

				UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSystem.BroadcastMessage(AbilityMessage.MessageType, AbilityMessage);

				// Broadcast the message to clients, since ApplyCooldown is only called on the server.
				if (ACrashGameState_DEP* GS = Cast<ACrashGameState_DEP>(UGameplayStatics::GetGameState(GetWorld())))
				{
					GS->MulticastReliableAbilityMessageToClients(AbilityMessage);
				}
			}
		}
	}
}

void UCrashGameplayAbilityBase::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Optional blueprint implementation of this callback.
	K2_InputReleased();

	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UCrashGameplayAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Optional blueprint implementation of this callback.
	K2_OnGiveAbility();
}

void UCrashGameplayAbilityBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	// Optional blueprint implementation of this callback.
	K2_OnRemoveAbility();
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