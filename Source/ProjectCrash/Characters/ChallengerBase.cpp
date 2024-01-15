// Copyright Samuel Reitich 2024.

#include "ChallengerBase.h"

#include "Input/CrashInputComponent.h"
#include "Input/CrashInputActionMapping.h"

AChallengerBase::AChallengerBase(const FObjectInitializer& ObjectInitializer)
{
	// Use the custom input component.
	OverrideInputComponentClass = UCrashInputComponent::StaticClass();
}

void AChallengerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Cache a reference to this character's input component as a CrashInputComponent.
	CrashInputComponent = Cast<UCrashInputComponent>(PlayerInputComponent);
	
	/* Bind the native input actions from each default action mapping to handler functions. We can't guarantee which
	 * action mapping each action will be in, so we have to check each one. */
	CrashInputComponent->BindInputAction(DefaultActionMapping, TAG_InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Mouse);
	CrashInputComponent->BindInputAction(DefaultActionMapping, TAG_InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Stick);
	CrashInputComponent->BindInputAction(DefaultActionMapping, TAG_InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
}

void AChallengerBase::Input_Look_Mouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y);
	}
}

void AChallengerBase::Input_Look_Stick(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	// Get how long the stick has been held.
	const UWorld* World = GetWorld();
	check(World);
	const float DeltaTime = World->GetDeltaSeconds();

	/* Scale the input action value based on the look-rate and the time the stick has been held before applying it to
	 * this character's controller. */
	if (Value.X != 0.0f)
	{
		// TODO: Add look-rate multiplier.
		AddControllerYawInput(Value.X * DeltaTime);
	}

	if (Value.Y != 0.0f)
	{
		// TODO: Add look-rate multiplier.
		AddControllerPitchInput(Value.Y * DeltaTime);
	}
}

void AChallengerBase::Input_Move(const FInputActionValue& InputActionValue)
{
	if (const AController* Controller = GetController())
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			// Get the movement direction relative to the world.
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}
