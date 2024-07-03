// Copyright Samuel Reitich. All rights reserved.


#include "Animation/CharacterAnimInstanceBase.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"


UCharacterAnimInstanceBase::UCharacterAnimInstanceBase() :
	WorldLocation(FVector::ZeroVector),
	WorldRotation(FRotator::ZeroRotator),
	YawDeltaSpeed(0.0f),
	SignedSpeed(0.0f),
	WorldVelocity(FVector::ZeroVector),
	LocalVelocity2D(FVector::ZeroVector),
	LocalVelocityDirectionAngle(0.0f),
	bHasVelocity(false),
	AimRotation(FRotator::ZeroRotator),
	bIsOnGround(false),
	bIsJumping(false),
	bIsFalling(false),
	TimeToJumpApex(0.0f),
	UpperBodyAdditiveWeight(0.0f)
{
	// Enable multithreading for animation updates.
	bUseMultiThreadedAnimationUpdate = true;
}

void UCharacterAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	// We can't safely update our animation data without a valid pawn owner.
	if (!TryGetPawnOwner())
	{
		return;
	}

	// Update animation data.
	UpdateTransformData(DeltaSeconds);
	UpdateVelocityData(DeltaSeconds);
	UpdateAimData(DeltaSeconds);
	UpdateCharacterStateData(DeltaSeconds);
	UpdateBlendData(DeltaSeconds);
}

void UCharacterAnimInstanceBase::UpdateTransformData(float DeltaSeconds)
{
	APawn* PawnOwner = TryGetPawnOwner();

	// World location.
    WorldLocation = PawnOwner->GetActorLocation();

	// World rotation.
	float PreviousYaw = WorldRotation.Yaw;
	WorldRotation = PawnOwner->GetActorRotation();

	// Yaw delta speed.
	const float YawDelta = WorldRotation.Yaw - PreviousYaw;
	YawDeltaSpeed = (YawDelta * SafeInvertDeltaSeconds(DeltaSeconds));
}

void UCharacterAnimInstanceBase::UpdateVelocityData(float DeltaSeconds)
{
	APawn* PawnOwner = TryGetPawnOwner();

	// World velocity.
	WorldVelocity = PawnOwner->GetVelocity();
	FVector WorldVelocity2D = WorldVelocity * FVector(1.0f, 1.0f, 0.0f);

	// Signed speed.
	SignedSpeed = WorldVelocity.Length();

	// Local velocity.
	LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);
	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);

	// Has velocity?
	bHasVelocity = !(FMath::IsNearlyZero(WorldVelocity2D.SquaredLength()));
}

void UCharacterAnimInstanceBase::UpdateAimData(float DeltaSeconds)
{
	// Aim rotation.
	AimRotation = TryGetPawnOwner()->GetBaseAimRotation();
}

void UCharacterAnimInstanceBase::UpdateCharacterStateData(float DeltaSeconds)
{
	APawn* PawnOwner = TryGetPawnOwner();
	UCharacterMovementComponent* MovementComp = Cast<UCharacterMovementComponent>(PawnOwner->GetMovementComponent());

	// On ground?
	bIsOnGround = MovementComp->IsMovingOnGround();

	// Jumping/falling?
	bIsJumping = false;
	bIsFalling = false;

	if (MovementComp->MovementMode == MOVE_Falling)
	{
		if (WorldVelocity.Z > 0.0f)
		{
			bIsJumping = true;
		}
		else
		{
			bIsFalling = true;
		}
	}

	// Time to jump apex.
	if (bIsJumping)
	{
		TimeToJumpApex = (0.0f - WorldVelocity.Z) / MovementComp->GetGravityZ();
	}
	else
	{
		TimeToJumpApex = 0.0f;
	}
}

void UCharacterAnimInstanceBase::UpdateBlendData(float DeltaSeconds)
{
	// Blend in the additive upper body slot when a montage is playing.
	if (IsAnyMontagePlaying())
	{
		UpperBodyAdditiveWeight = 1.0f;
	}
	// Smoothly blend out the upper body slot when montages end.
	else
	{
		UpperBodyAdditiveWeight = FMath::FInterpTo(UpperBodyAdditiveWeight, 0.0f, DeltaSeconds, UpperBodyBlendOutWeight);
	}
}
