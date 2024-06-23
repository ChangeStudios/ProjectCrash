// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"

UCrashCharacterMovementComponent::UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
	GravityScale = 1.5;
	MaxAcceleration = 16384.0f;
	BrakingFriction = 4.0f;
	bUseSeparateBrakingFriction = true;

	GroundFriction = 0.0f;
	MaxWalkSpeed = 800.0f;
	MaxWalkSpeedCrouched = 400.0f;
	BrakingDecelerationWalking = 8192.0f;

	JumpZVelocity = 700.0f;
	AirControl = 0.75f;
	FallingLateralFriction = 0.4;
}

void UCrashCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Disable separate braking friction while airborne.
	bUseSeparateBrakingFriction = (MovementMode != MOVE_Falling);
}
