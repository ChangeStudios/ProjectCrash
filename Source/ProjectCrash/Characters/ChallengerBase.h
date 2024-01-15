// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ChallengerBase.generated.h"

class UCrashInputActionMapping;
class UCrashInputComponent;
class UEnhancedInputUserSettings;

UCLASS()
class PROJECTCRASH_API AChallengerBase : public ACharacter
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AChallengerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Input.

// Components.
protected:

	// This character's input component cast to CrashInputComponent.
	UPROPERTY()
	TObjectPtr<UCrashInputComponent> CrashInputComponent;

// Action mappings.
protected:

	/** The action mapping that will be bound by default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UCrashInputActionMapping> DefaultActionMapping;

// User settings.
protected:

	/** Default user settings. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TArray<TObjectPtr<UEnhancedInputUserSettings>> DefaultUserSettings;

// Initialization.
public:

	/** Sets up input configurations, bindings, and settings. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

// Handler functions for native input events.
protected:

	/** Look input using a mouse. */
	void Input_Look_Mouse(const FInputActionValue& InputActionValue);

	/** Look input using a gamepad stick. */
	void Input_Look_Stick(const FInputActionValue& InputActionValue);

	/** Applies movement input. */
	void Input_Move(const FInputActionValue& InputActionValue);
};
