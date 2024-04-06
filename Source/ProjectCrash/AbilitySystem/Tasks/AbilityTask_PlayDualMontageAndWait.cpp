// Copyright Samuel Reitich 2024.


#include "AbilityTask_PlayDualMontageAndWait.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/CrashCharacterBase.h"


void UAbilityTask_PlayDualMontageAndWait::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// Check if this ability is currently playing this task's third-person montage.
	if ( (Montage == ThirdPersonMontageToPlay) && (Ability) && (Ability->GetCurrentMontage() == ThirdPersonMontageToPlay) )
	{
		// Reset the root motion translation scale.
		ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
		if (Character && (Character->GetLocalRole() == ROLE_Authority ||
							( (Character->GetLocalRole() == ROLE_AutonomousProxy) &&
								(Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)
							)))
		{
			Character->SetAnimRootMotionTranslationScale(1.0f);
		}

		// Notify the ASC that this ability is no longer animating the avatar.
		if (bInterrupted || !bAllowInterruptAfterBlendOut)
		{
			if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
			{
				ASC->ClearAnimatingAbility(Ability);
			}
		}
	}

	// Broadcast delegates to kismet.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast();

			// End this task after the montage finishes blending out after being interrupted.
			EndTask();
		}
		else
		{
			OnBlendOut.Broadcast();
		}
	}
}

void UAbilityTask_PlayDualMontageAndWait::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/* Broadcast to kismet that this task was completed when the montage finishes playing. If the montage was
	 * interrupted, this will be handled by OnGameplayAbilityCancelled instead. */
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast();
		}
	}

	EndTask();
}

void UAbilityTask_PlayDualMontageAndWait::OnGameplayAbilityCancelled()
{
	// If this ability is cancelled, stop any ongoing montages and end this task.
	if (StopPlayingMontage() || bAllowInterruptAfterBlendOut)
	{
		// Broadcast the interrupt to kismet.
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast();
		}
	}

	// Clean up delegates.
	OnCompleted.Clear();
	OnBlendOut.Clear();
	OnInterrupted.Clear();
	OnCancelled.Clear();

	EndTask();
}

void UAbilityTask_PlayDualMontageAndWait::ExternalCancel()
{
	// Broadcast to kismet that this task was cancelled.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast();
	}
	
	Super::ExternalCancel();
}

void UAbilityTask_PlayDualMontageAndWait::OnDestroy(bool AbilityEnded)
{
	// Stop any ongoing montages when this ability ends.
	if (Ability)
	{
		// Clear the OnInterrupted delegate.
		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);

		// Stop ongoing montages.
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	Super::OnDestroy(AbilityEnded);
}

UAbilityTask_PlayDualMontageAndWait* UAbilityTask_PlayDualMontageAndWait::CreatePlayDualMontageAndWaitProxy(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* FirstPersonMontageToPlay,
	UAnimMontage* ThirdPersonMontageToPlay, float FirstPersonRate, float ThirdPersonRate, FName FirstPersonStartSection,
	FName ThirdPersonStartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale,
	float FirstPersonStartTimeSeconds, float ThirdPersonStartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(ThirdPersonRate);

	// Create the ability task.
	UAbilityTask_PlayDualMontageAndWait* MyObj = NewAbilityTask<UAbilityTask_PlayDualMontageAndWait>(OwningAbility, TaskInstanceName);

	// Set the new task's parameters.
	MyObj->FirstPersonMontageToPlay = FirstPersonMontageToPlay;
	MyObj->ThirdPersonMontageToPlay = ThirdPersonMontageToPlay;
	MyObj->FirstPersonRate = FirstPersonRate;
	MyObj->ThirdPersonRate = ThirdPersonRate;
	MyObj->FirstPersonStartSection = FirstPersonStartSection;
	MyObj->ThirdPersonStartSection = ThirdPersonStartSection;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->FirstPersonStartTimeSeconds = FirstPersonStartTimeSeconds;
	MyObj->ThirdPersonStartTimeSeconds = ThirdPersonStartTimeSeconds;
	MyObj->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;

	return MyObj;
}

void UAbilityTask_PlayDualMontageAndWait::Activate()
{
	if (Ability == nullptr)
	{
		return;
	}

	bool bPlayedFirstPersonMontage = false;
	bool bPlayedThirdPersonMontage = false;

	if (UCrashAbilitySystemComponent* CrashASC = AbilitySystemComponent.Get() ? Cast<UCrashAbilitySystemComponent>(AbilitySystemComponent.Get()) : nullptr)
	{
		UAnimInstance* FirstPersonAnimInstance = nullptr;
		UAnimInstance* ThirdPersonAnimInstance = nullptr;

		// Try to retrieve the first- and third-person animation instances via the ACrashCharacterBase interface.
		AActor* Avatar = Ability->GetCurrentActorInfo()->AvatarActor.Get();
		if (ACrashCharacterBase* CrashAvatar = Avatar ? Cast<ACrashCharacterBase>(Avatar) : nullptr)
		{
			const USkeletalMeshComponent* FPPMesh = CrashAvatar->GetFirstPersonMesh();
			FirstPersonAnimInstance = FPPMesh ? FPPMesh->GetAnimInstance() : nullptr;

			const USkeletalMeshComponent* TPPMesh = CrashAvatar->GetThirdPersonMesh();
			ThirdPersonAnimInstance = TPPMesh ? TPPMesh->GetAnimInstance() : nullptr;
		}

		/* If the avatar is not a CrashCharacterBase, try to retrieve just the third-person animation instance by
		 * searching for any skeletal mesh it has. */
		if (ThirdPersonAnimInstance == nullptr)
		{
			const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
			ThirdPersonAnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
		}

		/* This task requires at least a valid third-person animation instance to function properly. If the avatar does
		 * not have one, this task does nothing. */
		if (ThirdPersonAnimInstance == nullptr)
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait: PlayDualMontageAndWait task was called on [%s], but failed to retrieve a third-person animation instance."), *GetNameSafe(AbilitySystemComponent.Get()));
			return;
		}

		// Play the third-person montage.
		bPlayedThirdPersonMontage = CrashASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), ThirdPersonMontageToPlay, ThirdPersonRate, ThirdPersonStartSection, ThirdPersonStartTimeSeconds) > 0.0f;

		// Play the third-person montage if the avatar has a valid first-person animation instance.
		bPlayedFirstPersonMontage = FirstPersonAnimInstance != nullptr ? CrashASC->PlayFirstPersonMontage(Ability, Ability->GetCurrentActivationInfo(), FirstPersonMontageToPlay, FirstPersonRate, FirstPersonStartSection, FirstPersonStartTimeSeconds) > 0.0f : false;

		// If either montage was successfully played, handle any consequences they could have on ability logic.
		if (bPlayedFirstPersonMontage || bPlayedThirdPersonMontage)
		{
			// Playing a montage could potentially fire off a callback into game code which could kill this ability.
			if (ShouldBroadcastAbilityTaskDelegates() == false)
			{
				return;
			}

			// Bind ability logic to the third-person montage.
			if (bPlayedThirdPersonMontage)
			{
				// Bind the OnInterrupted callback to when this ability is cancelled.
				InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnGameplayAbilityCancelled);

				// Bind the OnMontageBlendingOut callback to when the third-person montage finishes blending out.
				BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnMontageBlendingOut);
				ThirdPersonAnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ThirdPersonMontageToPlay);

				// Bind the OnMontageEnded callback to when the third-person montage ends.
				MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnMontageEnded);
				ThirdPersonAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ThirdPersonMontageToPlay);
			}
			else
			{
				ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait call to PlayThirdPersonMontage failed."));
			}

			// Update the root motion translation scale if we have authority over this ability.
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
							  ( (Character->GetLocalRole() == ROLE_AutonomousProxy) &&
							  	(Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)
							  )))
			{
				Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait calls to PlayFirstPersonMontage and PlayThirdPersonMontage failed."));
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait called on an invalid ASC."));
	}

	// Cancel this task if neither montage was played successfully.
	if (!bPlayedFirstPersonMontage && !bPlayedThirdPersonMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait called in ability [%s] failed to play montages [%s] and [%s]. Task Instance Name: [%s]."), *Ability->GetName(), *GetNameSafe(FirstPersonMontageToPlay), *GetNameSafe(ThirdPersonMontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast();
		}
	}

	SetWaitingOnAvatar();
}

bool UAbilityTask_PlayDualMontageAndWait::StopPlayingMontage()
{
	if (Ability == nullptr)
	{
		return false;
	}

	UAnimInstance* FirstPersonAnimInstance = nullptr;
	UAnimInstance* ThirdPersonAnimInstance = nullptr;

	// Try to retrieve the first- and third-person animation instances via the ACrashCharacterBase interface.
	const AActor* Avatar = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	const ACrashCharacterBase* CrashAvatar = Avatar ? Cast<ACrashCharacterBase>(Avatar) : nullptr;
	if (CrashAvatar)
	{
		FirstPersonAnimInstance = CrashAvatar->GetFirstPersonMesh() ? CrashAvatar->GetFirstPersonMesh()->GetAnimInstance() : nullptr;
		ThirdPersonAnimInstance = CrashAvatar->GetThirdPersonMesh() ? CrashAvatar->GetThirdPersonMesh()->GetAnimInstance() : nullptr;
	}

	/* If the avatar is not a CrashCharacterBase, try to retrieve just the third-person animation instance by searching
	 * for any skeletal mesh it has. */
	if (ThirdPersonAnimInstance == nullptr)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		ThirdPersonAnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
	}

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		// Stop playing the third-person montage.
		if (ThirdPersonAnimInstance)
		{
			if (ASC->GetAnimatingAbility() == Ability
				&& ASC->GetCurrentMontage() == ThirdPersonMontageToPlay)
			{
				// Unbind redundant delegates before stopping the montages.
				FAnimMontageInstance* ThirdPersonMontageInstance = ThirdPersonAnimInstance->GetActiveInstanceForMontage(ThirdPersonMontageToPlay);
				if (ThirdPersonMontageInstance)
				{
					ThirdPersonMontageInstance->OnMontageBlendingOutStarted.Unbind();
					ThirdPersonMontageInstance->OnMontageEnded.Unbind();
				}

				/* Stop the montage through the ASC to automatically notify it that this ability is no longer animating
				 * the avatar. */
				ASC->CurrentMontageStop();

				return true;
			}
		}

		// Stop playing the first-person montage.
		if (FirstPersonAnimInstance)
		{
			FAnimMontageInstance* FirstPersonMontageInstance = FirstPersonAnimInstance->GetActiveInstanceForMontage(FirstPersonMontageToPlay);
			if (FirstPersonMontageInstance)
			{
				FirstPersonMontageInstance->Stop(FirstPersonMontageToPlay->BlendOut, true);

				return true;
			}
		}
	}

	return false;
}

FString UAbilityTask_PlayDualMontageAndWait::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;

	if (Ability)
	{
		// The ASC's GetAnimInstance function should return the third-person montage if it's currently being played.
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(ThirdPersonMontageToPlay) ? ToRawPtr(ThirdPersonMontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayDualMontageAndWait. ThirdPersonMontageToPlay: [%s] (Currently Playing): [%s]"), *GetNameSafe(ThirdPersonMontageToPlay), *GetNameSafe(PlayingMontage));
}