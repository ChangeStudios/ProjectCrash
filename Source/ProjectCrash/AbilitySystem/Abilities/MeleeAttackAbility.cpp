// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/MeleeAttackAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayCueFunctionLibrary.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector_Capsule.h"
#include "AbilitySystem/Tasks/AbilityTask_PlayDualMontageAndWait.h"
#include "AbilitySystem/Tasks/AbilityTask_WaitReusableTargetData.h"
#include "BlueprintLibraries/AbilitySystemUtilitiesLibrary.h"
#include "Camera/CameraComponent.h"
#include "Characters/ChallengerBase.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"


UMeleeAttackAbility::UMeleeAttackAbility(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	AttackRange(150.0f),
	AttackRadius(25.0f),
	bUseInstantTargeting(true),
	IgnoreTargetsWithTags(FGameplayTagContainer()),
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


	// Set up the target actor, if we're using on. We only use target actors for duration-based detection.
	if (!bUseInstantTargeting)
	{
		// Determine where to attach the target actor.
		USceneComponent* TargetActorAttach;
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
			UE_LOG(LogTemp, Error, TEXT("Creating new target actor..."));
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
				IgnoreTargetsWithTags,
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


		// Start listening for when we should try to hit a surface.
		UAbilityTask_WaitGameplayEvent* SurfaceImpactTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this,CrashGameplayTags::TAG_Event_Ability_MeleeSurfaceImpact, nullptr, true, true);
		SurfaceImpactTask->EventReceived.AddDynamic(this, &UMeleeAttackAbility::TryHitSurface);
		SurfaceImpactTask->ReadyForActivation();
	}

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


	// If instant targeting is being used, start waiting for the triggering event.
	if (bUseInstantTargeting)
	{
		UAbilityTask_WaitGameplayEvent* PerformTargetingTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,CrashGameplayTags::TAG_Event_Ability_PerformTargeting, nullptr, true, true);
		PerformTargetingTask->EventReceived.AddDynamic(this, &UMeleeAttackAbility::OnPerformTargetingReceived);
		PerformTargetingTask->ReadyForActivation();
	}
	// If duration-based targeting is being used, immediately start targeting.
	else
	{
		StartTargeting();
	}


	// Update internal trackers.
	LastUsed = World->GetTimeSeconds();
	CurrentAttackAnim_FPP = CurrentAttackAnim_FPP >= (AttackMontages_FPP.Num() - 1) ? 0 : CurrentAttackAnim_FPP + 1;
	CurrentAttackAnim_TPP = CurrentAttackAnim_TPP >= (AttackMontages_TPP.Num() - 1) ? 0 : CurrentAttackAnim_TPP + 1;
}

void UMeleeAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// End targeting if we're using duration-based targeting.
	if (!bUseInstantTargeting)
	{
		EndTargeting();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMeleeAttackAbility::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& Data)
{
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
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor.Get()))
				{
					bHitTargets = true;

					// Play a "hit impact" gameplay cue, if desired.
					if (HitImpactCue.IsValid())
					{
						UAbilitySystemComponent* OwningASC = GetAbilitySystemComponentFromActorInfo_Checked();
						FHitResult ImpactHit;

						/* Try to trace straight forward along the target actor to the point of impact. This is more
						 * accurate than the target actor's default hit result, which comes from an overlap, instead of
						 * a trace. */
						FVector Start = OwningASC->GetAvatarActor()->GetActorLocation();
						FVector Forward = OwningASC->GetAvatarActor()->GetActorForwardVector();

						// If the ability user is a pawn, trace from their camera instead of their root.
						if (const APawn* Pawn = Cast<APawn>(OwningASC->GetAvatarActor()))
						{
							Start = Pawn->GetPawnViewLocation();
							const FRotator Rot = Pawn->GetBaseAimRotation();
							Forward = FRotationMatrix::Make(Rot).GetUnitAxis(EAxis::X);
						}
						const FVector End = Start + (Forward * AttackRange);

						// Try to perform a line trace for the highest accuracy.
						FCollisionQueryParams QueryParams;
						QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
						GetWorld()->LineTraceSingleByChannel(ImpactHit, Start, End, ECC_Camera, QueryParams);

						/* If the line trace did not hit our target, trace around the target actor. This should always
						 * succeed, since the target actor itself already found a hit. */
						if (!(ImpactHit.bBlockingHit && ImpactHit.GetActor() == HitActor))
						{
							UKismetSystemLibrary::CapsuleTraceSingle
							(
								this,
								Start,
								End,
								AttackRadius,
								AttackRange / 2.0f,
								UEngineTypes::ConvertToTraceType(ECC_Camera),
								true,
								TArray<AActor*>({GetAvatarActorFromActorInfo()}),
								EDrawDebugTrace::None,
								ImpactHit,
								true
							);
						}

						// Play the impact cue.
						if (ImpactHit.bBlockingHit && ImpactHit.GetActor() == HitActor)
						{
							FGameplayEffectContextHandle Context = OwningASC->MakeEffectContext();
							Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());
							Context.AddOrigin(ImpactHit.ImpactPoint);
							Context.AddHitResult(ImpactHit);
							TargetASC->ExecuteGameplayCue(HitImpactCue, Context);
						}
					}

					// Call blueprint implementation for hitting a target, which will handle applying gameplay effects. 
					OnTargetHit(TargetASC);
				}
			}
		}
	}
}

void UMeleeAttackAbility::OnPerformTargetingReceived(FGameplayEventData Payload)
{
	FVector Start;
	FVector End;
	GetCapsulePosition(false, Start, End);
	// The capsule's root is always halfway between its top and base.

	TArray<FHitResult> Hits;

	// When the PerformTargeting event is received, perform targeting via capsule collision trace.
	UKismetSystemLibrary::SphereTraceMulti
	(
		this,
		Start,
		End,
		AttackRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		true,
		TArray<AActor*>({GetAvatarActorFromActorInfo()}),
		EDrawDebugTrace::None,
		Hits,
		true
	);

	// Send targeting data for each hit.
	TArray<AActor*> HitActors;
	for (FHitResult Result : Hits)
	{
		if (AActor* ResultActor = Result.GetActor())
		{
			// Make sure actors aren't accidentally hit multiple times.
			if (HitActors.Contains(ResultActor))
			{
				continue;
			}

			HitActors.Add(ResultActor);

			// Filter for ability system actors.
			const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ResultActor);
			if (!ASC || ASC->HasAnyMatchingGameplayTags(IgnoreTargetsWithTags))
			{
				continue;
			}

			// Simulate target data being received to handle hit targets.
			bHitTargets = true;
			FGameplayAbilityTargetingLocationInfo StartLocation = FGameplayAbilityTargetingLocationInfo();
			OnTargetDataReceived(StartLocation.MakeTargetDataHandleFromActors( TArray<TWeakObjectPtr<AActor>>({ResultActor}) ));
		}
	}

	// If we didn't hit any valid targets, try hitting a surface instead.
	if (!bHitTargets)
	{
		TryHitSurface(FGameplayEventData());
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
	WaitTargetDataTask->ValidDataSentDelegate.AddDynamic(this, &UMeleeAttackAbility::OnTargetDataReceived);

	WaitTargetDataTask->ReadyForActivation();
}

void UMeleeAttackAbility::EndTargeting()
{
	// Stop the target actor's targeting and end the waiting-for-data task.
	if (TargetActor)
	{
		TargetActor->StopTargeting();
		EndTaskByInstanceName(FName("WaitTargetDataTask"));
	}
}

void UMeleeAttackAbility::TryHitSurface(FGameplayEventData Payload)
{
	if (CurrentActorInfo->IsNetAuthority() && SurfaceImpactCue.IsValid())
	{
		UAbilitySystemComponent* OwningASC = GetAbilitySystemComponentFromActorInfo();
		if (!OwningASC || !OwningASC->GetAvatarActor())
		{
			return;
		}

		// Perform a line trace to find a surface to hit.
		FHitResult SurfaceHit;
		FVector Start;
		FVector End;
		GetCapsulePosition(true, Start, End);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
		GetWorld()->LineTraceSingleByChannel(SurfaceHit, Start, End, ECC_Visibility, QueryParams);

		// If we hit something, play the surface hit cue on it.
		if (SurfaceHit.bBlockingHit)
		{
			FGameplayEffectContextHandle Context = OwningASC->MakeEffectContext();
			Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());
			Context.AddOrigin(SurfaceHit.ImpactPoint);
			Context.AddHitResult(SurfaceHit);
			OwningASC->ExecuteGameplayCue(SurfaceImpactCue, Context);
		}
	}
}

void UMeleeAttackAbility::GetCapsulePosition(bool bIncludeRadius, FVector& Base, FVector& Top)
{
	const UAbilitySystemComponent* OwningASC = GetAbilitySystemComponentFromActorInfo();

	// Use the avatar's location and rotation as the default capsule position.
	FVector BaseWithRadius = OwningASC->GetAvatarActor()->GetActorLocation();
	FVector Forward = OwningASC->GetAvatarActor()->GetActorForwardVector();

	// If the avatar is a player character, use their camera instead.
	if (const AChallengerBase* AvatarAsChallenger = Cast<AChallengerBase>(OwningASC->GetAvatarActor()))
	{
		BaseWithRadius = AvatarAsChallenger->GetFirstPersonCameraComponent()->GetComponentLocation();
		const FRotator Rot = AvatarAsChallenger->GetControlRotation();
		Forward = FRotationMatrix::Make(Rot).GetUnitAxis(EAxis::X);
	}

	const FVector TopWithRadius = BaseWithRadius + (Forward * AttackRange);
	const FVector BaseWithoutRadius = BaseWithRadius + (Forward * AttackRadius);
	const FVector TopWithoutRadius  = TopWithRadius - (Forward * AttackRadius);

	Base = bIncludeRadius ? BaseWithRadius : BaseWithoutRadius;
	Top = bIncludeRadius ? TopWithRadius : TopWithoutRadius;
}

void UMeleeAttackAbility::EndAbilityWrapper()
{
	// End this ability.
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
