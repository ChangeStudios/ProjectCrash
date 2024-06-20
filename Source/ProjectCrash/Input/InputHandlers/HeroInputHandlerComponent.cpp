// Copyright Samuel Reitich. All rights reserved.


#include "HeroInputHandlerComponent.h"

#include "CrashGameplayTags.h"
#include "Input/CrashInputComponent.h"

void UHeroInputHandlerComponent::BindInputHandlers(UCrashInputComponent* CrashIC, const UCrashInputActionMapping* ActionMapping)
{
	Super::BindInputHandlers(CrashIC, ActionMapping);

	// Bind this component's native action handlers.
	CrashIC->BindNativeInputAction(ActionMapping, CrashGameplayTags::TAG_InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	CrashIC->BindNativeInputAction(ActionMapping, CrashGameplayTags::TAG_InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Mouse);
	CrashIC->BindNativeInputAction(ActionMapping, CrashGameplayTags::TAG_InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Stick);
}

void UHeroInputHandlerComponent::Input_Look_Mouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	// Yaw input.
	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	// Pitch input.
	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UHeroInputHandlerComponent::Input_Look_Stick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}

	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	// Yaw input.
	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * BaseLookRate_Yaw * World->GetDeltaSeconds());
	}

	// Pitch input.
	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * BaseLookRate_Pitch * World->GetDeltaSeconds());
	}
}

void UHeroInputHandlerComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		// Right/left input.
		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		// Forward/backward input.
		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}
