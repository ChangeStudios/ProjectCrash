// Copyright Samuel Reitich 2024.


#include "Animation/ChallengerAnimInstanceBase.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/ChallengerBase.h"
#include "GameFramework/CrashLogging.h"

void UChallengerAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Enable multi-threading for updating animations.
	bUseMultiThreadedAnimationUpdate = true;
}

void UChallengerAnimInstanceBase::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// Cache the owning challenger character.
	OwningChallenger = TryGetPawnOwner() ? Cast<AChallengerBase>(TryGetPawnOwner()) : nullptr;
	ANIMATION_LOG(Warning, TEXT("Begin play called for [%s] on server: %s"), *GetNameSafe(GetOwningActor()), *CONDITIONAL_STRING(OwningChallenger->HasAuthority()));

	// This animation instance won't work without a valid Challenger.
	if (!IsValid(OwningChallenger))
	{
		ANIMATION_LOG(Fatal, TEXT("OwningChallenger not valid in animation instance [%s] for [%s]. The animation system will not work properly."), *GetName(), *GetNameSafe(TryGetPawnOwner()));
		bUseMultiThreadedAnimationUpdate = false;
	}

	// Cache this animation's owning pawn's ASC when it's initialized.
	OwningChallenger->ASCInitializedDelegate.AddDynamic(this, &UChallengerAnimInstanceBase::OnASCInitialized);
}

bool UChallengerAnimInstanceBase::ThreadSafeHasTagExact(UAbilitySystemComponent* ASC, FGameplayTag TagToSearch) const
{
	if (!IsValid(ASC))
	{
		return false;
	}

	return ASC->GetGameplayTagCount(TagToSearch) > 0;
}

void UChallengerAnimInstanceBase::OnASCInitialized(UCrashAbilitySystemComponent* CrashASC)
{
	ANIMATION_LOG(Warning, TEXT("ASC Initialized on %s"), *FString(OwningChallenger->HasAuthority() ? "SERVER" : "CLIENT"));

	if (!CrashASC)
	{
		ANIMATION_LOG(Fatal, TEXT("OnASCInitialized broadcast to [%s] without a valid ASC."), *GetName());
	}

	OwningASC = CrashASC;
}

void UChallengerAnimInstanceBase::UpdateAnimData(UCharacterAnimData* NewAnimData)
{
	if (NewAnimData)
	{
		CurrentAnimationData = NewAnimData;
	}
}

void UChallengerAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateMovementVelocity();
}

void UChallengerAnimInstanceBase::UpdateMovementVelocity()
{
	// Cache the pawn's current movement values.
	if (IsValid(OwningChallenger))
	{
		const FVector PawnVelocity = OwningChallenger->GetVelocity();
		SignedSpeed = OwningChallenger->GetVelocity().Length();
		
		const FVector UnrotatedVelocity = OwningChallenger->GetActorRotation().UnrotateVector(PawnVelocity);
		ForwardBackwardSpeed = UnrotatedVelocity.X;
		RightLeftSpeed = UnrotatedVelocity.Y;
		UpDownSpeed = UnrotatedVelocity.Z;
	}
}
