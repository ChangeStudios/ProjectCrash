// Copyright Samuel Reitich. All rights reserved.


#include "Animation/CharacterAnimInstanceBase.h"

#include "Characters/CrashCharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetMathLibrary.h"

/** The rate at which we blend out the additive upper body slot. I.e. the speed at which UpperBodyAdditiveWeight is
 * lerped back to 0 when montages end. */
#define UPPER_BODY_BLEND_OUT_RATE 6.0f

UCharacterAnimInstanceBase::UCharacterAnimInstanceBase() :
	bFirstUpdate(true),
	WorldLocation(FVector::ZeroVector),
	WorldRotation(FRotator::ZeroRotator),
	YawDeltaSpeed(0.0f),
	WorldVelocity(FVector::ZeroVector),
	LocalVelocity2D(FVector::ZeroVector),
	LocalVelocity2DNormalized(FVector::ZeroVector),
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

	// Wait for our movement component and movement mode to be initialized.
	UCrashCharacterMovementComponent* CharMovementComp = GetCrashCharacterMovementComponent();
	if (!CharMovementComp || (CharMovementComp->MovementMode == MOVE_None))
	{
		return;
	}

	// Update animation data.
	UpdateTransformData(DeltaSeconds);
	UpdateVelocityData(DeltaSeconds);
	UpdateAimData(DeltaSeconds);
	UpdateCharacterStateData(DeltaSeconds);
	UpdateBlendData(DeltaSeconds);

	// We've successfully made our first update.
	if (bFirstUpdate)
	{
		bFirstUpdate = false;
	}
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
	const float YawDelta = (bFirstUpdate ? 0.0f : (WorldRotation.Yaw - PreviousYaw));
	YawDeltaSpeed = (YawDelta * SafeInvertDeltaSeconds(DeltaSeconds));
}

void UCharacterAnimInstanceBase::UpdateVelocityData(float DeltaSeconds)
{
	APawn* PawnOwner = TryGetPawnOwner();
	UCrashCharacterMovementComponent* CharMovementComp = GetCrashCharacterMovementComponent();

	// World velocity.
	WorldVelocity = PawnOwner->GetVelocity();
	const FVector WorldVelocity2D = WorldVelocity * FVector(1.0f, 1.0f, 0.0f);

	// Local velocity.
	LocalVelocity = WorldRotation.UnrotateVector(WorldVelocity);
	LocalVelocity2D = LocalVelocity * FVector(1.0f, 1.0f, 0.0f);
	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);

	// Normalized local velocity.
	const float MaxMovementSpeed = CharMovementComp->GetMaxSpeed();
	const float NormalizedX = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.X, 0.0f, MaxMovementSpeed);
	const float NormalizedY = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.Y, 0.0f, MaxMovementSpeed);
	LocalVelocity2DNormalized = FVector(NormalizedX, NormalizedY, 0.0f);

	// Has velocity?
	bHasVelocity = !(FMath::IsNearlyZero(WorldVelocity2D.SquaredLength()));
}

void UCharacterAnimInstanceBase::UpdateAimData(float DeltaSeconds)
{
	// Aim rotation.
	AimRotation = TryGetPawnOwner()->GetBaseAimRotation();
	AimRotation.Pitch = FRotator::NormalizeAxis(AimRotation.Pitch); // Replication inconsistency fix.
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
		UpperBodyAdditiveWeight = FMath::FInterpTo(UpperBodyAdditiveWeight, 0.0f, DeltaSeconds, UPPER_BODY_BLEND_OUT_RATE);
	}
}

UCrashCharacterMovementComponent* UCharacterAnimInstanceBase::GetCrashCharacterMovementComponent() const
{
	if (APawn* PawnOwner = TryGetPawnOwner())
	{
		if (UPawnMovementComponent* MovementComp = PawnOwner->GetMovementComponent())
		{
			if (UCrashCharacterMovementComponent* CharMovementComp = Cast<UCrashCharacterMovementComponent>(MovementComp))
			{
				return CharMovementComp;
			}
		}
	}

	return nullptr;
}