// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/MeleeAttackAbilityDEP.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "DisplayDebugHelpers.h"
#include "GameplayCueFunctionLibrary.h"
#include "KismetTraceUtils.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector_Capsule.h"
#include "AbilitySystem/Tasks/AbilityTask_PlayDualMontageAndWait.h"
#include "AbilitySystem/Tasks/AbilityTask_WaitTargetDataWithReusableActor.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/HUD.h"
#include "Kismet/KismetSystemLibrary.h"

#define USING_DURATION_TARGETING TargetingType == EMeleeTargetingType::EventDuration || TargetingType == EMeleeTargetingType::EntireDuration

UMeleeAttackAbilityDEP::UMeleeAttackAbilityDEP(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	AttackRange(150.0f),
	AttackRadius(25.0f),
	TargetingType(EMeleeTargetingType::EntireDuration),
	IgnoreTargetsWithTags(FGameplayTagContainer()),
	TargetActor(nullptr),
	LastUsed(0.0f),
	TimeBeforeReset(0.0f),
	CurrentAttackAnim_FPP(0),
	CurrentAttackAnim_TPP(0),
	bHitTargets(false),
	bGASDebugEnabled(false)
{
}

void UMeleeAttackAbilityDEP::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// Track when the GAS debugger is enabled.
#if WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
	AHUD::OnShowDebugInfo.AddWeakLambda(this, [this] (AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
	{
		bGASDebugEnabled = DisplayInfo.IsDisplayOn(TEXT("AbilitySystem"));
	});
#endif // WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
}

void UMeleeAttackAbilityDEP::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	UWorld* const World = GetWorld();
	check(ASC && World);


	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	// For instant targeting, set up target data generation.
	if (TargetingType == EMeleeTargetingType::Instant)
	{
		OnTargetDataReadyDelegateHandle = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReady);
	}
	// For duration-based targeting, set up the collision detector target actor.
	else if (USING_DURATION_TARGETING)
	{
		// Determine where to attach the target actor.
		// TODO: Fix
		USceneComponent* TargetActorAttach;
		// if (const AChallengerBase* AvatarAsChallenger = Cast<AChallengerBase>(ASC->GetAvatarActor()))
		// {
		// 	// If the avatar is a player character, use their camera.
		// 	TargetActorAttach = AvatarAsChallenger->GetFirstPersonCameraComponent();
		// }
		// else
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
				FName("CapsuleHitDetection"),
				true,
				false,
				true,
				FGameplayTargetDataFilterHandle(),
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

		// Start listening for when we should try to hit a surface.
		UAbilityTask_WaitGameplayEvent* SurfaceImpactTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this,CrashGameplayTags::TAG_GameplayEvent_Ability_MeleeSurfaceImpact, nullptr, true, true);
		SurfaceImpactTask->EventReceived.AddDynamic(this, &UMeleeAttackAbilityDEP::TryHitSurface);
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

	AnimTask->BlendOutDelegate.AddDynamic(this, &UMeleeAttackAbilityDEP::K2_EndAbility);
	AnimTask->InterruptedDelegate.AddDynamic(this, &UMeleeAttackAbilityDEP::K2_EndAbility);
	AnimTask->CancelledDelegate.AddDynamic(this, &UMeleeAttackAbilityDEP::K2_EndAbility);
	AnimTask->ReadyForActivation();


	// If instant targeting is being used, start waiting for the triggering event.
	if (TargetingType == EMeleeTargetingType::Instant)
	{
		// Perform targeting on the client.
		if (IsLocallyControlled())
		{
			UAbilityTask_WaitGameplayEvent* PerformTargetingTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, CrashGameplayTags::TAG_GameplayEvent_Ability_PerformTargeting, nullptr, true, true);
			PerformTargetingTask->EventReceived.AddDynamic(this, &UMeleeAttackAbilityDEP::PerformInstantTargeting);
			PerformTargetingTask->ReadyForActivation();
		}
	}
	// If duration-based targeting is being used, immediately start targeting.
	else if (TargetingType == EMeleeTargetingType::EntireDuration)
	{
		StartTargeting();
	}


	// Update internal trackers.
	LastUsed = World->GetTimeSeconds();
	CurrentAttackAnim_FPP = CurrentAttackAnim_FPP >= (AttackMontages_FPP.Num() - 1) ? 0 : CurrentAttackAnim_FPP + 1;
	CurrentAttackAnim_TPP = CurrentAttackAnim_TPP >= (AttackMontages_TPP.Num() - 1) ? 0 : CurrentAttackAnim_TPP + 1;
}

void UMeleeAttackAbilityDEP::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Consume target data and remove the delegate for instant targeting when this ability ends.
	if (TargetingType == EMeleeTargetingType::Instant)
	{
		if (IsEndAbilityValid(Handle, ActorInfo))
		{
			/* We might be locked out of ending this ability while our other prediction scope is active. Wait until the
			 * scope ends to properly end this ability. */
			if (ScopeLockCount > 0)
			{
				WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
				return;
			}

			UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
			check(ASC);

			ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyDelegateHandle);
			ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
		}
	}
	// End targeting if we're using duration-based targeting.
	else if (USING_DURATION_TARGETING)
	{
		EndTargeting();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMeleeAttackAbilityDEP::PerformInstantTargeting(FGameplayEventData Payload)
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);

	// Create a new prediction window for generating target data.
	FScopedPredictionWindow ScopedPrediction(ASC, CurrentActivationInfo.GetActivationPredictionKey());

	// Perform the trace.
	TArray<FHitResult> Hits;
	PerformTrace(Hits);

	// Generate target data from the hit result.
	FGameplayAbilityTargetDataHandle TargetData = MakeTargetLocationInfoFromOwnerActor().MakeTargetDataHandleFromHitResults(this, Hits);

	// Immediately process the target data.
	OnTargetDataReady(TargetData, FGameplayTag());
}

bool UMeleeAttackAbilityDEP::PerformTrace(TArray<FHitResult>& OutHitResults) const
{
	/* There won't be a target actor if we're doing a trace, but we can calculate where the target actor's capsule
	 * would be if we had one. This makes instant targeting and duration-based targeting behave identically, and
	 * takes the capsule radius into account for our sphere trace. */
	FVector TraceStart;
	FVector TraceEnd;
	GetCapsulePosition(false, TraceStart, TraceEnd);

	// Perform the trace.
	TArray<FHitResult> Hits;
	UKismetSystemLibrary::SphereTraceMulti
	(
		this,
		TraceStart,
		TraceEnd,
		AttackRadius,
		UEngineTypes::ConvertToTraceType(ECC_Camera),
		true,
		TArray<AActor*>({GetAvatarActorFromActorInfo()}),
		EDrawDebugTrace::None,
		Hits,
		true
	);

	// Draw debug line if GAS debugging is enabled.
#if WITH_EDITOR
	const EDrawDebugTrace::Type DebugTraceType = EDrawDebugTrace::ForDuration;
	const float DebugTraceDuration = 2.0f;

	if (bGASDebugEnabled)
	{
		DrawDebugSphereTraceMulti
		(
			GetWorld(),
			TraceStart,
			TraceEnd,
			AttackRadius,
			DebugTraceType,
			Hits.Num() > 0,
			Hits,
			FLinearColor::Red,
			FLinearColor::Green,
			DebugTraceDuration
		);
	}
#endif // WITH_EDITOR

	// Perform filtering.
	TArray<AActor*> HitActors;
	for (FHitResult& Hit : Hits)
	{
		// Prevent the same actor from being hit multiple times.
		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActors.Contains(HitActor))
			{
				continue;
			}
			else
			{
				HitActors.Add(HitActor);
			}

			// If the hit target has an ASC, check if it has any ignored tags.
			if (const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Hit.GetActor()))
			{
				if (ASC->HasAnyMatchingGameplayTags(IgnoreTargetsWithTags))
				{
					continue;
				}
			}
		}

		OutHitResults.Add(Hit);
	}

	return OutHitResults.Num() > 0;
}

void UMeleeAttackAbilityDEP::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(ASC);

	if (const FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		// New prediction window while we wait for the server to receive the target data.
		FScopedPredictionWindow ScopedPrediction(ASC);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us.
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		// Notify the server if necessary.
		if (CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
		{
			ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, ASC->ScopedPredictionKey);
		}

		// Perform logic with the target data once confirmed.
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			OnTargetDataReceived(LocalTargetDataHandle);
		}
		else
		{
			K2_EndAbility();
		}
	}

	// We've processed the data.
	ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UMeleeAttackAbilityDEP::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData)
{
	// Perform ability logic on each hit target.
	for (TSharedPtr<FGameplayAbilityTargetData> TargetData : InData.Data)
	{
		// Try to retrieve the hit actor from the target data hit result. Only valid with instant targeting.
		TArray<AActor*> HitActors;
		AActor* HitResultActor = TargetData->HasHitResult() ? TargetData->GetHitResult()->GetActor() : nullptr;
		if (HitResultActor)
		{
			HitActors.Add(HitResultActor);
		}
		/* If there is no hit result (i.e. duration-based targeting is being used), collect hit actors directly from
		 * the target data. */
		else
		{
			for (TWeakObjectPtr<AActor> TargetDataActor : TargetData->GetActors())
			{
				HitActors.Add(TargetDataActor.Get());
			}
		}

		for (AActor* HitActor : HitActors)
		{
			// TODO: Check for line-of-sight.
			const bool bHasLOS = true;

			if (bHasLOS)
			{
				/* Retrieve the hit actor's ASC. */
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor))
				{
					bHitTargets = true;

					// Determine where we hit.
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

					// If we still somehow don't have a valid point to play our cue, just use the hit actor.
					if (!(ImpactHit.bBlockingHit && ImpactHit.GetActor() == HitActor))
					{
						ImpactHit = FHitResult
						(
							HitActor,
							nullptr,
							HitActor->GetActorLocation(),
							FVector()
						);
						ImpactHit.bBlockingHit = true;

						ABILITY_LOG(Verbose, TEXT("MeleeAttackAbility %s failed to generate a hit for triggering effects."), *GetName());
					}

					// Trigger the hit effects.
					if (ImpactHit.bBlockingHit && ImpactHit.GetActor() == HitActor)
					{
						FGameplayEffectContextHandle Context = OwningASC->MakeEffectContext();
						Context.AddHitResult(ImpactHit);

						// Play an impact cue, if desired.
						if (DefaultHitImpactCue.IsValid())
						{
							OwningASC->ExecuteGameplayCue(DefaultHitImpactCue, UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(ImpactHit));
						}

						// Call blueprint implementation for hitting a target, which will handle applying gameplay effects. 
						OnTargetHit(TargetASC, Context);
					}
				}
			}
		}
	}

	/* If we didn't hit any valid targets, try hitting a surface instead. Other targeting types trigger this from
	 * events. */
	if (!bHitTargets && TargetingType == EMeleeTargetingType::Instant)
	{
		TryHitSurface(FGameplayEventData());
	}
}

void UMeleeAttackAbilityDEP::StartTargeting()
{
	if (!TargetActor)
	{
		ABILITY_LOG(Error, TEXT("UMeleeAttackAbility: StartTargeting was called without a valid target actor! Avatar: %s"), *GetNameSafe(CurrentActorInfo->AvatarActor.Get()));
		return;
	}

	/* Start waiting for target data. This is done predictively, which can trigger a warning like "LogPredictionKey:
	 * Warning: UnAck'd PredictionKey...". This warning is a bug that should be fixed in future UE versions. */
	UAbilityTask_WaitTargetDataWithReusableActor* WaitTargetDataTask = UAbilityTask_WaitTargetDataWithReusableActor::CreateWaitTargetDataWithReusableActorProxy
	(
		this,
		FName("WaitTargetDataTask"),
		EGameplayTargetingConfirmation::CustomMulti,
		TargetActor
	);
	WaitTargetDataTask->TargetDataReadyDelegate.AddDynamic(this, &UMeleeAttackAbilityDEP::OnTargetDataReceived);
	WaitTargetDataTask->ReadyForActivation();
}

void UMeleeAttackAbilityDEP::EndTargeting()
{
	if (!TargetActor)
	{
		ABILITY_LOG(Error, TEXT("UMeleeAttackAbility: StartTargeting was called without a valid target actor! Avatar: %s"), *GetNameSafe(CurrentActorInfo->AvatarActor.Get()));
		return;
	}

	// Stop the target actor's targeting and end the waiting-for-data task, if the target actor is currently targeting.
	TargetActor->StopTargeting();
	CancelTaskByInstanceName(FName("WaitTargetDataTask"));
}

void UMeleeAttackAbilityDEP::TryHitSurface(FGameplayEventData Payload)
{
	if (DefaultSurfaceImpactCue.IsValid())
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
			// FGameplayEffectContextHandle Context = OwningASC->MakeEffectContext();
			// Context.AddInstigator(GetOwningActorFromActorInfo(), GetAvatarActorFromActorInfo());
			// Context.AddOrigin(SurfaceHit.ImpactPoint);
			OwningASC->ExecuteGameplayCue(DefaultSurfaceImpactCue, UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(SurfaceHit));
		}
	}
}

void UMeleeAttackAbilityDEP::GetCapsulePosition(bool bIncludeRadius, FVector& Base, FVector& Top) const
{
	const UAbilitySystemComponent* OwningASC = GetAbilitySystemComponentFromActorInfo();

	// Use the avatar's location and rotation as the default capsule position.
	FVector BaseWithRadius = OwningASC->GetAvatarActor()->GetActorLocation();
	FVector Forward = OwningASC->GetAvatarActor()->GetActorForwardVector();

	// TODO: Fix
	// If the avatar is a player character, use their camera instead.
	// if (const AChallengerBase* AvatarAsChallenger = Cast<AChallengerBase>(OwningASC->GetAvatarActor()))
	// {
	// 	BaseWithRadius = AvatarAsChallenger->GetFirstPersonCameraComponent()->GetComponentLocation();
	// 	const FRotator Rot = AvatarAsChallenger->GetControlRotation();
	// 	Forward = FRotationMatrix::Make(Rot).GetUnitAxis(EAxis::X);
	// }

	const FVector TopWithRadius = BaseWithRadius + (Forward * AttackRange);
	const FVector BaseWithoutRadius = BaseWithRadius + (Forward * AttackRadius);
	const FVector TopWithoutRadius  = TopWithRadius - (Forward * AttackRadius);

	Base = bIncludeRadius ? BaseWithRadius : BaseWithoutRadius;
	Top = bIncludeRadius ? TopWithRadius : TopWithoutRadius;
}