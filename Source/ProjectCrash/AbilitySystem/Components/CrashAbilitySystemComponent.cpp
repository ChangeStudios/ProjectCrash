// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "Characters/CrashCharacterBase.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"

UCrashAbilitySystemComponent::UCrashAbilitySystemComponent()
{
	CurrentExclusiveAbility = nullptr;
}

void UCrashAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// Register this ASC with the global ability system.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->RegisterASC(this);
	}

	// Broadcast that this ASC was initialized.
	InitDelegate.Broadcast(InOwnerActor, InAvatarActor);
}

void UCrashAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Unregister this ASC from the global ability system.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->UnregisterASC(this);
	}
}

void UCrashAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// Broadcast that a new ability was granted to this ASC.
	AbilityGrantedDelegate.Broadcast(AbilitySpec);
}

void UCrashAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	// Broadcast that an ability was removed from this ASC.
	AbilityRemovedDelegate.Broadcast(AbilitySpec);
}

bool UCrashAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup ActivationGroup) const
{
	// Independent abilities are never blocked.
	if (ActivationGroup == EAbilityActivationGroup::Independent)
	{
		return false;
	}

	// If there is no active exclusive ability, then no exclusive abilities will be blocked.
	if (!IsValid(CurrentExclusiveAbility))
	{
		return false;
	}

	// New exclusive abilities are blocked if the active exclusive ability is a blocking ability.
	return CurrentExclusiveAbility->GetActivationGroup() == EAbilityActivationGroup::Exclusive_Blocking;
}

void UCrashAbilitySystemComponent::HandleAbilityActivatedForActivationGroup(UCrashGameplayAbilityBase* ActivatedAbility)
{
	switch (ActivatedAbility->GetActivationGroup())
	{
		// We don't need to do anything for independent abilities.
		case EAbilityActivationGroup::Independent:
		{
			return;
		}

		/* If the activated ability is exclusive, cancel the active replaceable exclusive ability, if there is one and
		 * cache the new exclusive ability */
		case EAbilityActivationGroup::Exclusive_Replaceable:
		case EAbilityActivationGroup::Exclusive_Blocking:
		{
			if (CurrentExclusiveAbility)
			{
				/* If there is an active exclusive ability, it must be replaceable (otherwise the given ability
				 * wouldn't have been activated). */
				if (CurrentExclusiveAbility->GetActivationGroup() == EAbilityActivationGroup::Exclusive_Replaceable)
				{
					CurrentExclusiveAbility->CancelAbility(CurrentExclusiveAbility->GetCurrentAbilitySpecHandle(), CurrentExclusiveAbility->GetCurrentActorInfo(), CurrentExclusiveAbility->GetCurrentActivationInfo(), true);
				}
				/* If there is a current exclusive ability but it is not exclusive and replaceable, then it is either
				 * independent (and should not have been cached as the exclusive ability) or it is blocking (and the
				 * given ability should not have been activated). */
				else
				{
					ABILITY_LOG(Warning, TEXT("UCrashAbilitySystemComponent: A replaceable ability, [%s], was activated, but the current exclusive ability was not successfully overridden."), *GetNameSafe(ActivatedAbility));
					return;
				}
			}

			// Cache the new exclusive ability.
			CurrentExclusiveAbility = ActivatedAbility;
		}
	}
}

void UCrashAbilitySystemComponent::HandleAbilityEndedForActivationGroup(UCrashGameplayAbilityBase* EndedAbility)
{
	// If the given ability is the current exclusive ability, clear it from the exclusive ability cache.
	if (CurrentExclusiveAbility == EndedAbility)
	{
		CurrentExclusiveAbility = nullptr;
	}
}

void UCrashAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	// Send a standardized message that this ability was activated.
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		FCrashAbilityMessage AbilityMessage = FCrashAbilityMessage();
		AbilityMessage.MessageType = CrashGameplayTags::TAG_Message_Ability_Activated;
		AbilityMessage.ActorInfo = *AbilityActorInfo;

		if (const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
		{
			// Observers expect the failed ability's CDO.
			AbilityMessage.Ability = CrashAbility->GetAbilityCDO();
		}
		else
		{
			AbilityMessage.Ability = Ability;
		}

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(AbilityMessage.MessageType, AbilityMessage);
	}
}

void UCrashAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	// Send a standardized message that this ability ended.
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		FCrashAbilityMessage AbilityMessage = FCrashAbilityMessage();
		AbilityMessage.MessageType = CrashGameplayTags::TAG_Message_Ability_Ended;
		AbilityMessage.ActorInfo = *AbilityActorInfo;

		if (const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
		{
			// Observers expect the failed ability's CDO.
			AbilityMessage.Ability = CrashAbility->GetAbilityCDO();
		}
		else
		{
			AbilityMessage.Ability = Ability;
		}

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(AbilityMessage.MessageType, AbilityMessage);
	}
}

void UCrashAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	// Send a standardized message that the ability activation failed.
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		FCrashAbilityMessage AbilityMessage = FCrashAbilityMessage();
		AbilityMessage.MessageType = CrashGameplayTags::TAG_Message_Ability_Failed;
		AbilityMessage.ActorInfo = *AbilityActorInfo;

		if (const UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
		{
			// Observers expect the failed ability's CDO.
			AbilityMessage.Ability = CrashAbility->GetAbilityCDO();
		}
		else
		{
			AbilityMessage.Ability = Ability;
		}

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(AbilityMessage.MessageType, AbilityMessage);
	}
}

float UCrashAbilitySystemComponent::PlayFirstPersonMontage(UGameplayAbility* InAnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName, float StartTimeSeconds)
{
	float Duration = -1.0f;

	/* Get the first-person mesh's animation instance, if there is one. This can only be retrieved via the
	 * ACrashCharacterBase interface. */
	UAnimInstance* AnimInstance = nullptr;
	if (const ACrashCharacterBase* CrashAvatar = GetAvatarActor() ? Cast<ACrashCharacterBase>(GetAvatarActor()) : nullptr)
	{
		AnimInstance = CrashAvatar->GetFirstPersonMesh() ? CrashAvatar->GetFirstPersonMesh()->GetAnimInstance() : nullptr;
	}

	if (AnimInstance && NewAnimMontage)
	{
		// Play the montage on the first-person animation instance.
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);

		if (Duration > 0.f)
		{
			UAnimSequenceBase* Animation = NewAnimMontage->IsDynamicMontage() ? NewAnimMontage->GetFirstAnimReference() : NewAnimMontage;

			if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UCrashAbilitySystemComponent::PlayFirstPersonMontage [%s], Role: [%s]")
					, *GetNameSafe(NewAnimMontage)
					, *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole())
					);
			}

			// Start the montage at the given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, NewAnimMontage);
			}

			/* Replicate for non-owners and for replay recordings. The data we set from GetRepAnimMontageInfo_Mutable()
			 * is used both by the server to replicate to clients and by clients to record replays. We need to set this
			 * data for recording clients because there exists network configurations where an ability's montage data
			 * will not replicate to some clients (e.g. if the client is an autonomous proxy). */
			if (ShouldRecordMontageReplication())
			{
				FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo = GetRepAnimMontageInfo_Mutable();

				// Static parameters. These are set when the montage is played and are never changed afterwards.
				MutableRepAnimMontageInfo.Animation = Animation;
				MutableRepAnimMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0);

				MutableRepAnimMontageInfo.SectionIdToPlay = 0;
				if (MutableRepAnimMontageInfo.Animation && StartSectionName != NAME_None)
				{
					// Add one so INDEX_NONE can be used in the OnRep.
					MutableRepAnimMontageInfo.SectionIdToPlay = NewAnimMontage->GetSectionIndex(StartSectionName) + 1;
				}

				// Update replicated data that changed during the montage's lifetime.
				AnimMontage_UpdateReplicatedData();
			}

			// Replicate to non-owners.
			if (IsOwnerActorAuthoritative())
			{
				// Force net update on the avatar actor.
				if (AbilityActorInfo->AvatarActor != nullptr)
				{
					AbilityActorInfo->AvatarActor->ForceNetUpdate();
				}
			}
			else
			{
				// If this prediction key is rejected, we need to stop the predicted montage.
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					PredictionKey.NewRejectedDelegate().BindUObject(this, &UCrashAbilitySystemComponent::OnFirstPersonPredictiveMontageRejected, NewAnimMontage);
				}
			}
		}
	}

	return Duration;
}

void UCrashAbilitySystemComponent::OnFirstPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	// Retrieve the first-person animation instance. This can only be retrieved via the ACrashCharacterBase interface.
	UAnimInstance* AnimInstance = nullptr;
	if (const ACrashCharacterBase* CrashAvatar = GetAvatarActor() ? Cast<ACrashCharacterBase>(GetAvatarActor()) : nullptr)
	{
		AnimInstance = CrashAvatar->GetFirstPersonMesh() ? CrashAvatar->GetFirstPersonMesh()->GetAnimInstance() : nullptr;
	}

	if (AnimInstance && PredictiveMontage)
	{
		// If this montage is still playing, stop it.
		if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
		{
			AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
		}
	}
}