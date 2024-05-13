// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/MeleeAttackAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector_Capsule.h"
#include "AbilitySystem/Tasks/AbilityTask_PlayDualMontageAndWait.h"
#include "AbilitySystem/Tasks/AbilityTask_WaitReusableTargetData.h"
#include "BlueprintLibraries/AbilitySystemUtilitiesLibrary.h"
#include "Camera/CameraComponent.h"
#include "Characters/ChallengerBase.h"


UMeleeAttackAbility::UMeleeAttackAbility(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	AttackRange(150.0f),
	AttackRadius(25.0f),
	bDetectEntireDuration(true),
	TargetActor(nullptr),
	LastUsed(0.0f),
	TimeBeforeReset(0.0f),
	CurrentAttackAnim_FPP(0),
	CurrentAttackAnim_TPP(0),
	bHitTargets(false)
{
}

void UMeleeAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// This ability requires at least one first- and third-person attack montage.
	if (AttackMontages_FPP.Num() == 0 || AttackMontages_TPP.Num() == 0)
	{
#if WITH_EDITOR
		ABILITY_LOG(Error, TEXT("UMeleeAttackAbility: %s: Missing a first- and third-person attack montage. Ability canceled."), *GetName());
		CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false);
#elif (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		check(AttackMontages_FPP.Num() > 0 && AttackMontages_TPP.Num() > 0)
#endif // WITH_EDITOR
	}

	// There should be an equal amount of first- and third-person attack montages, but this will not break the ability.
	if (AttackMontages_FPP.Num() != AttackMontages_TPP.Num())
	{
		ABILITY_LOG(Warning, TEXT("UMeleeAttackAbility: %s: First- and Third-Person Attack Montages arrays are of different lengths; this may cause mismatched visuals!"), *GetName());
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* const World = GetWorld();
	check(ASC && World);


	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	// Determine where to attach the target actor.
	USceneComponent* TargetActorAttach = nullptr;
	if (const AChallengerBase* AvatarAsChallenger = Cast<AChallengerBase>(ASC->GetAvatarActor()))
	{
		// If the avatar is a player character, use their camera.
		TargetActorAttach = AvatarAsChallenger->GetFirstPersonCameraComponent();
	}
	else
	{
		// If the avatar is not a player, use their root component.
		TargetActorAttach = ASC->GetAvatarActor()->GetRootComponent();
	}

	// Create the target actor, if it has not been created yet.
	if (!IsValid(TargetActor))
	{
		TargetActor = World->SpawnActor<AGameplayAbilityTargetActor_CollisionDetector_Capsule>(AGameplayAbilityTargetActor_CollisionDetector_Capsule::StaticClass());

		check(TargetActor);

		// Configure the target actor.
		TargetActor->Configure
		(
			AttackRadius,
			AttackRange / 2.0f,
			true,
			false,
			nullptr,
			true,
			false
		);
	}

	// Re-attach the target actor to the source component, in case we switched avatars.
	TargetActor->AttachToComponent(TargetActorAttach, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	// The target actor is attached at its root, which is its center. We need to shift it so it's attached at its base.
	TargetActor->SetActorRelativeLocation(FVector(AttackRange / 2.0f, 0.0f, 0.0f));
	TargetActor->SetActorRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	// Reset the hit targets with each activation.
	TargetActor->ResetTargets();
	bHitTargets = false;


	// Reset the current attack sequence if this attack is not immediately after our previous one.
	const float LastAttackEndTime = LastUsed + FMath::Max(AttackMontages_TPP[CurrentAttackAnim_TPP]->GetPlayLength(),
															AttackMontages_FPP[CurrentAttackAnim_FPP]->GetPlayLength());
	if (World->GetTimeSeconds() > ( LastAttackEndTime + TimeBeforeReset ))
	{
		CurrentAttackAnim_FPP = 0;
		CurrentAttackAnim_TPP = 0;
	}

	// Play the next first- and third-person attack animations in the sequence.
	UAbilityTask_PlayDualMontageAndWait* AnimTask = UAbilityTask_PlayDualMontageAndWait::CreatePlayDualMontageAndWaitProxy
	(
		this,
		NAME_None,
		AttackMontages_FPP[CurrentAttackAnim_FPP],
		AttackMontages_TPP[CurrentAttackAnim_TPP],
		1.0f,
		1.0f,
		NAME_None,
		NAME_None,
		false
	);

	AnimTask->OnBlendOut.AddDynamic(this, &UMeleeAttackAbility::EndAbilityWrapper);
	AnimTask->OnInterrupted.AddDynamic(this, &UMeleeAttackAbility::EndAbilityWrapper);
	AnimTask->OnCancelled.AddDynamic(this, &UMeleeAttackAbility::EndAbilityWrapper);
	AnimTask->ReadyForActivation();


	// If we should detect hits for the entire duration, then immediately start detecting.
	if (bDetectEntireDuration)
	{
		StartTargeting();
	}
	// If we are using events to control when to perform targeting, start listening for those events.
	else
	{
		// Start listening for when to start targeting.
		UAbilityTask_WaitGameplayEvent* StartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,CrashGameplayTags::TAG_Event_Ability_MeleeAttack_Start, nullptr, true, true);
		StartTask->EventReceived.AddDynamic(this, &UMeleeAttackAbility::OnEventReceived);


		// Start listening for when to end targeting.
		UAbilityTask_WaitGameplayEvent* EndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, CrashGameplayTags::TAG_Event_Ability_MeleeAttack_End, nullptr, true, true);
		EndTask->EventReceived.AddDynamic(this, &UMeleeAttackAbility::OnEventReceived);
	}


	// Update internal trackers.
	LastUsed = World->GetTimeSeconds();
	CurrentAttackAnim_FPP = CurrentAttackAnim_FPP >= (AttackMontages_FPP.Num() - 1) ? 0 : CurrentAttackAnim_FPP + 1;
	CurrentAttackAnim_TPP = CurrentAttackAnim_TPP >= (AttackMontages_TPP.Num() - 1) ? 0 : CurrentAttackAnim_TPP + 1;
}

void UMeleeAttackAbility::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data)
{
	UE_LOG(LogTemp, Error, TEXT("HIT"));
	for (TSharedPtr<FGameplayAbilityTargetData> TargetData : Data.Data)
	{
		for (TWeakObjectPtr<AActor> HitActor : TargetData->GetActors())
		{
			// Check for line-of-sight.
			const bool bHasLOS = UAbilitySystemUtilitiesLibrary::Actor_HasLineOfSight
			(
				this,
				GetAvatarActorFromActorInfo(),
				HitActor.Get(),
				UEngineTypes::ConvertToTraceType(ECC_Visibility), 
				true
			);

			if (bHasLOS)
			{
				/* Retrieve the hit actor's ASC. The target actor has already filtered for ASC actors for us. */
				if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor.Get()))
				{
					bHitTargets = true;

					// TODO: Trace to target and trigger cue.

					// Call blueprint implementation for hitting a target, which will handle applying gameplay effects. 
					OnTargetHit(ASC);
				}
			}
		}
	}

}

void UMeleeAttackAbility::OnEventReceived(FGameplayEventData Payload)
{
	// When the Start event is received, start targeting.
	if (Payload.EventTag == CrashGameplayTags::TAG_Event_Ability_MeleeAttack_Start)
	{
		StartTargeting();
	}
	// When the End event is received, end targeting.
	else if (Payload.EventTag == CrashGameplayTags::TAG_Event_Ability_MeleeAttack_End)
	{
		EndTargeting();
	}
}

void UMeleeAttackAbility::StartTargeting()
{
	// Wait for target data.
	UAbilityTask_WaitReusableTargetData* WaitTargetDataTask = UAbilityTask_WaitReusableTargetData::WaitTargetDataWithReusableActor
	(
		this,
		FName("WaitTargetDataTask"),
		EGameplayTargetingConfirmation::CustomMulti,
		TargetActor
	);
	WaitTargetDataTask->ReadyForActivation();

	// Start listening for sent target data.
	WaitTargetDataTask->ValidDataSentDelegate.AddDynamic(this, &UMeleeAttackAbility::OnTargetDataReceived);
}

void UMeleeAttackAbility::EndTargeting()
{
	// Cancel the target actor's targeting and end the waiting-for-data task.
	TargetActor->CancelTargeting();
	EndTaskByInstanceName(FName("WaitTargetDataTask"));

	// If we did not hit anything, we can check for a surface to hit instead.
	if (!bHitTargets)
	{
		// TODO: Trace for surface and fire cue.
	}
}

void UMeleeAttackAbility::EndAbilityWrapper()
{
	// End this ability.
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
