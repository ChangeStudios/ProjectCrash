// Copyright Samuel Reitich 2024.


#include "CrashGameplayAbilityBase.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "NativeGameplayTags.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Effects/CrashGameplayEffectContext.h"
#include "Characters/ChallengerBase.h"

UCrashGameplayAbilityBase::UCrashGameplayAbilityBase(const FObjectInitializer& ObjectInitializer)
{
	// I can't imagine a situation where the instancing policy *wouldn't* be "instanced per actor."
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityIcon = nullptr;

	if (InstancingPolicy != EGameplayAbilityInstancingPolicy::InstancedPerActor)
	{
		ABILITY_LOG(Warning, TEXT("Ability [%s] is not instanced-per-actor. Certain activation style features may not work properly."), *GetName());
	}
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
			bIsMutable = AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_AbilitySlot) ||
							AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_WeaponSlot) ||
								AbilityTags.HasTagExact(CrashGameplayTags::TAG_UI_AbilityBehavior_Ultimate);
		}
	}

	return bIsMutable;
}

#endif // #if WITH_EDITOR

void UCrashGameplayAbilityBase::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Optional blueprint implementation of this callback.
	K2_InputReleased();

	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UCrashGameplayAbilityBase::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// Optional blueprint implementation of this callback.
	K2_OnGiveAbility();

	Super::OnGiveAbility(ActorInfo, Spec);
}

void UCrashGameplayAbilityBase::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// Optional blueprint implementation of this callback.
	K2_OnRemoveAbility();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UCrashGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Apply this ability's applied gameplay effects to its ASC.
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		// Create an effect context with which to apply this ability's effects.
		const FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();

		// Apply every ongoing effect that will NOT be automatically removed when this ability ends.
		for (auto GE : OngoingEffectsToApplyOnStart)
		{
			if (!GE.Get())
			{
				continue;
			}

			// Make an outgoing spec handle for the effect.
			FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(GE, 1, EffectContextHandle);
			if (!EffectSpecHandle.IsValid())
			{
				continue;
			}

			// Apply the effect.
			FActiveGameplayEffectHandle ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
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
				FGameplayEffectSpecHandle EffectSpecHandle = ASC->MakeOutgoingSpec(GE, 1, EffectContextHandle);
				if (!EffectSpecHandle.IsValid())
				{
					continue;
				}

				// Apply the effect.
				FActiveGameplayEffectHandle ActiveEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

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
	}
}

void UCrashGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Before ending this ability, remove all ongoing effects that are marked to be removed when the ability ends.
	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : EffectsToRemoveOnEndHandles)
	{
		if (ActiveEffectHandle.IsValid())
		{
			ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
		}
	}

	// Reset the array of effect handles to remove on ability end.
	EffectsToRemoveOnEndHandles.Reset();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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