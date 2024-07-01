// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacterMovementComponent.h"

UCrashCharacterMovementComponent::UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
	GravityScale = 1.5;
	MaxAcceleration = 16384.0f;
	BrakingFriction = 4.0f;
	bUseSeparateBrakingFriction = true;

	GroundFriction = 0.0f;
	MaxWalkSpeed = 600.0f;
	MaxWalkSpeedCrouched = 300.0f;
	MinAnalogWalkSpeed = 50.0f;
	BrakingDecelerationWalking = 8192.0f;

	JumpZVelocity = 1000.0f;
	AirControl = 0.5f;
	AirControlBoostVelocityThreshold = 0.0f;
	FallingLateralFriction = 0.4;
}

void UCrashCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Disable separate braking friction while airborne.
	bUseSeparateBrakingFriction = (MovementMode != MOVE_Falling);
}
