// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "Characters/CrashCharacterBase.h"
#include "GameFramework/CrashLogging.h"

void UCrashAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// Register this ASC with the global ability system.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		GlobalAbilitySystem->RegisterASC(this);
	}
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
				MutableRepAnimMontageInfo.AnimMontage = NewAnimMontage;
				MutableRepAnimMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0);

				MutableRepAnimMontageInfo.SectionIdToPlay = 0;
				if (MutableRepAnimMontageInfo.AnimMontage && StartSectionName != NAME_None)
				{
					// Add one so INDEX_NONE can be used in the OnRep.
					MutableRepAnimMontageInfo.SectionIdToPlay = MutableRepAnimMontageInfo.AnimMontage->GetSectionIndex(StartSectionName) + 1;
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

float UCrashAbilitySystemComponent::PlayThirdPersonMontage(UGameplayAbility* InAnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* NewAnimMontage, float InPlayRate, FName StartSectionName, float StartTimeSeconds)
{
	float Duration = -1.0f;

	// Try to retrieve the third-person animation instance via the ACrashCharacterBase interface. 
	UAnimInstance* AnimInstance = nullptr;
	if (const ACrashCharacterBase* CrashAvatar = GetAvatarActor() ? Cast<ACrashCharacterBase>(GetAvatarActor()) : nullptr)
	{
		AnimInstance = CrashAvatar->GetThirdPersonMesh() ? CrashAvatar->GetThirdPersonMesh()->GetAnimInstance() : nullptr;
	}

	/* If the third-person animation instance couldn't be found through ACrashCharacterBase, fall back to any skeletal
	 * mesh component owned by the avatar. */
	if (AnimInstance == nullptr && AbilityActorInfo)
	{
		AnimInstance = AbilityActorInfo->AnimInstance.Get();
	}


	if (AnimInstance && NewAnimMontage)
	{
		// Play the montage on the third-person animation instance.
		Duration = AnimInstance->Montage_Play(NewAnimMontage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);

		if (Duration > 0.f)
		{
			if (NewAnimMontage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UCrashAbilitySystemComponent::PlayThirdPersonMontage [%s], Role: [%s]")
					, *GetNameSafe(NewAnimMontage)
					, *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole())
					);
			}

			// Save that the given ability is now animating this ASC. 
			LocalAnimMontageInfo.AnimMontage = NewAnimMontage;
			LocalAnimMontageInfo.AnimatingAbility = InAnimatingAbility;
			LocalAnimMontageInfo.PlayInstanceId = (LocalAnimMontageInfo.PlayInstanceId < UINT8_MAX ? LocalAnimMontageInfo.PlayInstanceId + 1 : 0);

			// Notify the ability that it is now animating this ASC.
			if (InAnimatingAbility)
			{
				InAnimatingAbility->SetCurrentMontage(NewAnimMontage);
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
				MutableRepAnimMontageInfo.AnimMontage = NewAnimMontage;
				MutableRepAnimMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0);

				MutableRepAnimMontageInfo.SectionIdToPlay = 0;
				if (MutableRepAnimMontageInfo.AnimMontage && StartSectionName != NAME_None)
				{
					// Add one so INDEX_NONE can be used in the OnRep.
					MutableRepAnimMontageInfo.SectionIdToPlay = MutableRepAnimMontageInfo.AnimMontage->GetSectionIndex(StartSectionName) + 1;
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
					PredictionKey.NewRejectedDelegate().BindUObject(this, &UCrashAbilitySystemComponent::OnThirdPersonPredictiveMontageRejected, NewAnimMontage);
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

void UCrashAbilitySystemComponent::OnThirdPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	// Try to retrieve the third-person animation instance via the ACrashCharacterBase interface. 
	UAnimInstance* AnimInstance = nullptr;
	if (const ACrashCharacterBase* CrashAvatar = GetAvatarActor() ? Cast<ACrashCharacterBase>(GetAvatarActor()) : nullptr)
	{
		AnimInstance = CrashAvatar->GetThirdPersonMesh() ? CrashAvatar->GetThirdPersonMesh()->GetAnimInstance() : nullptr;
	}

	/* If the third-person animation instance couldn't be found through ACrashCharacterBase, fall back to any skeletal
	 * mesh component owned by the avatar. */
	if (AnimInstance == nullptr && AbilityActorInfo)
	{
		AnimInstance = AbilityActorInfo->AnimInstance.Get();
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
