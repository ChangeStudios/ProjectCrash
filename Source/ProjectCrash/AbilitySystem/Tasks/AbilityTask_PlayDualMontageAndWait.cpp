// Copyright Samuel Reitich. All rights reserved.


#include "AbilityTask_PlayDualMontageAndWait.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/CrashCharacter.h"

UAbilityTask_PlayDualMontageAndWait* UAbilityTask_PlayDualMontageAndWait::CreatePlayDualMontageAndWaitProxy(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* FirstPersonMontageToPlay,
	UAnimMontage* ThirdPersonMontageToPlay, float FirstPersonRate, float ThirdPersonRate, FName FirstPersonStartSection,
	FName ThirdPersonStartSection, bool bStopWhenAbilityEnds, float AnimRootMotionTranslationScale,
	float FirstPersonStartTimeSeconds, float ThirdPersonStartTimeSeconds, bool bAllowInterruptAfterBlendOut)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(ThirdPersonRate);

	// Create the ability task.
	UAbilityTask_PlayDualMontageAndWait* MyObj = NewAbilityTask<UAbilityTask_PlayDualMontageAndWait>(OwningAbility, TaskInstanceName);

	// Set the task's parameters.
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

	bool bPlayedThirdPersonMontage = false;

	if (UCrashAbilitySystemComponent* CrashASC = AbilitySystemComponent.IsValid() ? Cast<UCrashAbilitySystemComponent>(AbilitySystemComponent.Get()) : nullptr)
	{
		// Retrieve the first-person and third-person animation instances.
		const FCrashGameplayAbilityActorInfo* CrashActorInfo = CastChecked<UCrashGameplayAbilityBase>(Ability)->GetCrashActorInfo();
		UAnimInstance* FirstPersonAnimInstance = CrashActorInfo->GetFirstPersonAnimInstance();
		UAnimInstance* ThirdPersonAnimInstance = CrashActorInfo->GetAnimInstance();

		/* This task requires a third-person animation instance. If the avatar does not have one, this task does
		 * nothing. */
		if (ThirdPersonAnimInstance == nullptr)
		{
			ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait: PlayDualMontageAndWait task was called by [%s], but failed to find a third-person animation instance."), *GetNameSafe(Ability));
			return;
		}

		// Play the first-person montage if the avatar has a valid first-person animation instance.
		FirstPersonAnimInstance != nullptr ? CrashASC->PlayMontage_FirstPerson(Ability, Ability->GetCurrentActivationInfo(), FirstPersonMontageToPlay, FirstPersonRate, FirstPersonStartSection, FirstPersonStartTimeSeconds) > 0.0f : false;

		// Play the third-person montage.
		bPlayedThirdPersonMontage = CrashASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), ThirdPersonMontageToPlay, ThirdPersonRate, ThirdPersonStartSection, ThirdPersonStartTimeSeconds) > 0.0f;

		// If the third-person montage was successfully played, start listening for relevant gameplay events.
		if (bPlayedThirdPersonMontage)
		{
			// We only need to bind gameplay events if we want to broadcast delegates in response.
			if (ShouldBroadcastAbilityTaskDelegates() == false)
			{
				return;
			}

			// Bind OnInterrupted to when this ability is cancelled.
			InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnGameplayAbilityCancelled);

			// Bind OnMontageBlendingOut to when the third-person montage finishes blending out.
			BlendingOutDelegate.BindUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnMontageBlendingOut);
			ThirdPersonAnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ThirdPersonMontageToPlay);

			// Bind OnMontageEnded to when the third-person montage ends.
			MontageEndedDelegate.BindUObject(this, &UAbilityTask_PlayDualMontageAndWait::OnMontageEnded);
			ThirdPersonAnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ThirdPersonMontageToPlay);

			// Update the root motion translation scale, if we have authority over this ability.
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
							  ( (Character->GetLocalRole() == ROLE_AutonomousProxy) &&
							  	(Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)
							  )))
			{
				Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
			}
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait called without a valid ASC."));
	}


	// Cancel this task if the third-person montage was played successfully.
	if (!bPlayedThirdPersonMontage)
	{
		ABILITY_LOG(Warning, TEXT("UAbilityTask_PlayDualMontageAndWait called in ability [%s] failed to play third-person montage [%s] Task Instance Name: [%s]."), *GetNameSafe(Ability), *GetNameSafe(ThirdPersonMontageToPlay), *InstanceName.ToString());

		// Cancel the first-person montage if it successfully played.
		StopPlayingMontage();

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			CancelledDelegate.Broadcast();
		}
	}

	SetWaitingOnAvatar();
}

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

	// Trigger the appropriate output pin.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		// If the montage is blending out because it was interrupted, fire OnInterrupted.
		if (bInterrupted)
		{
			InterruptedDelegate.Broadcast();

			// End this task after the montage finishes blending out after being interrupted.
			EndTask();
		}
		// If the montage is blending out after its montage finished, fire OnBlendOut.
		else
		{
			BlendOutDelegate.Broadcast();
		}
	}
}

void UAbilityTask_PlayDualMontageAndWait::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/* Fire the OnCompleted pin when this montage ends. If the montage ended because it was interrupted, this will
	 * instead be handled by OnGameplayAbilityCancelled. */
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			CompletedDelegate.Broadcast();
		}
	}

	EndTask();
}

void UAbilityTask_PlayDualMontageAndWait::OnGameplayAbilityCancelled()
{
	// If this ability is cancelled, stop any ongoing montages and end this task.
	if (StopPlayingMontage() || bAllowInterruptAfterBlendOut)
	{
		// Fire the OnInterrupted pin.
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			InterruptedDelegate.Broadcast();
		}
	}

	// Clear delegates.
	CompletedDelegate.Clear();
	BlendOutDelegate.Clear();
	InterruptedDelegate.Clear();
	CancelledDelegate.Clear();

	EndTask();
}

void UAbilityTask_PlayDualMontageAndWait::ExternalCancel()
{
	// Fire the OnCancelled pin.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		CancelledDelegate.Broadcast();
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

		// Stop any montages currently playing, if desired.
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	Super::OnDestroy(AbilityEnded);
}

bool UAbilityTask_PlayDualMontageAndWait::StopPlayingMontage()
{
	if (Ability == nullptr)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return false;
	}

	// Retrieve the first-person and third-person animation instances.
	const FCrashGameplayAbilityActorInfo* CrashActorInfo = CastChecked<UCrashGameplayAbilityBase>(Ability)->GetCrashActorInfo();
	UAnimInstance* FirstPersonAnimInstance = CrashActorInfo->GetFirstPersonAnimInstance();
	UAnimInstance* ThirdPersonAnimInstance = CrashActorInfo->GetAnimInstance();

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

	// Print the montage currently playing on the third-person mesh when debugging this task.
	if (Ability)
	{
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		if (UAnimInstance* AnimInstance = ActorInfo ? ActorInfo->GetAnimInstance() : nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(ThirdPersonMontageToPlay) ? ToRawPtr(ThirdPersonMontageToPlay) : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayDualMontageAndWait. ThirdPersonMontageToPlay: [%s] (Currently Playing): [%s]"), *GetNameSafe(ThirdPersonMontageToPlay), *GetNameSafe(PlayingMontage));
}