// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ThirdPersonCharacterAnimInstance.h"

#if WITH_EDITORONLY_DATA
#include "KismetAnimationLibrary.h"
#include "Characters/CrashCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#endif

/** The dead-zone value used for converting this character's local velocity into a cardinal direction. */
#define CARDINAL_DEAD_ZONE 10.0f

/** The maximum speed with which the additive landing recovery animation will be scaled. When landing, if the vertical
 * velocity with which the character landed is less than this, the alpha of the recovery animation will be scaled down
 * against this value. E.g. landing with a vertical velocity of (MAX_FALLING_SPEED / 2) will only apply the recovery
 * animation with an alpha of 0.5. */
#define MAX_FALLING_SPEED 5000.0f



UThirdPersonCharacterAnimInstance::UThirdPersonCharacterAnimInstance() :
	GroundDistance(0.0f),
	GroundDistanceDelta(0.0f),
	LocalVelocityDirection(EAnimCardinalDirection::Forward),
	LandRecoveryAlpha(0.0f)
{
#if WITH_EDITORONLY_DATA
	Debug_WorldVelocity = FVector::ZeroVector;
	Debug_Pitch = 0.0f;
#endif
}

void UThirdPersonCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
#if WITH_EDITORONLY_DATA
	// Update the preview instance in the archetype.
	if (!GetWorld()->IsPlayInEditor())
	{
		UpdateDebugData(DeltaSeconds);
	}
#endif

	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}

void UThirdPersonCharacterAnimInstance::UpdateTransformData(float DeltaSeconds)
{
	Super::UpdateTransformData(DeltaSeconds);

	// Update this character's current distance from the ground.
	UCrashCharacterMovementComponent* CrashMovementComp = GetCrashCharacterMovementComponent();
	const float LastGroundDistance = GroundDistance;
	GroundDistance = CrashMovementComp ? CrashMovementComp->GetGroundDistance() : UCrashCharacterMovementComponent::MAX_FLOOR_DIST;
	GroundDistanceDelta = (GroundDistance - LastGroundDistance);
}

void UThirdPersonCharacterAnimInstance::UpdateVelocityData(float DeltaSeconds)
{
	// Cache the current velocity as the previous frame's velocity, before we update it.
	LastLocalVelocity = LocalVelocity;

	Super::UpdateVelocityData(DeltaSeconds);

	// Update which local cardinal direction this character is currently moving.
	LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle, CARDINAL_DEAD_ZONE, true, LocalVelocityDirection);

	// Update acceleration.
	const FVector VelocityDelta = (LocalVelocity - LastLocalVelocity);
	Acceleration = (VelocityDelta * SafeInvertDeltaSeconds(DeltaSeconds));
}

void UThirdPersonCharacterAnimInstance::UpdateCharacterStateData(float DeltaSeconds)
{
	const bool bWasAirborne = !bIsOnGround;

	Super::UpdateCharacterStateData(DeltaSeconds);

	// When landing, start applying the additive "land recovery" animation.
	if (bWasAirborne && bIsOnGround)
	{
		// Scale the initial recovery alpha using the vertical velocity at which the character landed.
		LandRecoveryAlpha = UKismetMathLibrary::MapRangeClamped(LastLocalVelocity.Z, 0.0f, -MAX_FALLING_SPEED, 0.1f, 1.0f);
	}
}

EAnimCardinalDirection UThirdPersonCharacterAnimInstance::SelectCardinalDirectionFromAngle(float LocalAngle, float DeadZone, bool bUseCurrentDirection, EAnimCardinalDirection CurrentDirection)
{
	const float AbsLocalAngle = FMath::Abs(LocalAngle);
	float ForwardDeadZone = DeadZone;
	float BackwardDeadZone = DeadZone;

	/* Increase the dead-zone of the current direction, if desired. This makes it more difficult to change from the
	 * current direction, so we don't rapidly switch between directions. */
	if (bUseCurrentDirection)
	{
		if (CurrentDirection == EAnimCardinalDirection::Forward)
		{
			ForwardDeadZone *= 2.0f;
		}
		else if (CurrentDirection == EAnimCardinalDirection::Backward)
		{
			BackwardDeadZone *= 2.0f;
		}
	}

	// Forward range.
	if (AbsLocalAngle <= (45.0f + ForwardDeadZone))
	{
		return EAnimCardinalDirection::Forward;
	}
	// Backward range.
	else if (AbsLocalAngle >= (135.0f - BackwardDeadZone))
	{
		return EAnimCardinalDirection::Backward;
	}
	// Right range.
	else if (LocalAngle > 0.0f)
	{
		return EAnimCardinalDirection::Right;
	}
	// Left range.
	else
	{
		return EAnimCardinalDirection::Left;
	}
}

#if WITH_EDITORONLY_DATA
void UThirdPersonCharacterAnimInstance::UpdateDebugData(float DeltaSeconds)
{
	AActor* PreviewActor = GetOwningActor();

	if (!PreviewActor)
	{
		UE_LOG(LogAnimation, Error, TEXT("Anim instance [%s] failed to find preview instance in debug."), *GetNameSafe(this));
		return;
	}

	// Velocity data.
	WorldVelocity = Debug_WorldVelocity;
	const FVector WorldVelocity2D = WorldVelocity * FVector(1.0f, 1.0f, 0.0f);
	LocalVelocity = WorldRotation.UnrotateVector(WorldVelocity);
	LocalVelocity2D = LocalVelocity * FVector(1.0f, 1.0f, 0.0f);
	LocalVelocityDirectionAngle = UKismetAnimationLibrary::CalculateDirection(WorldVelocity2D, WorldRotation);
	const float MaxMovementSpeed = UCrashCharacterMovementComponent::StaticClass()->GetDefaultObject<UCrashCharacterMovementComponent>()->MaxWalkSpeed;
	const float NormalizedX = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.X, 0.0f, MaxMovementSpeed);
	const float NormalizedY = UKismetMathLibrary::NormalizeToRange(LocalVelocity2D.Y, 0.0f, MaxMovementSpeed);
	LocalVelocity2DNormalized = FVector(NormalizedX, NormalizedY, 0.0f);
	bHasVelocity = !(FMath::IsNearlyZero(WorldVelocity2D.SquaredLength()));
	LocalVelocityDirection = SelectCardinalDirectionFromAngle(LocalVelocityDirectionAngle, CARDINAL_DEAD_ZONE, true, LocalVelocityDirection);

	// Aim data.
	AimRotation = WorldRotation;
	AimRotation.Pitch = Debug_Pitch;
}
#endif