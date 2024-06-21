// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/InputHandlers/InputHandlerComponentBase.h"
#include "HeroInputHandlerComponent.generated.h"

class UCrashInputActionMapping;

/**
 * Defines input for simple pawn movement.
 */
UCLASS(Meta = (BlueprintSpawnableComponent, ShortToolTip = "Sets up input for basic pawn movement. Requires a pawn extension component."))
class PROJECTCRASH_API UHeroInputHandlerComponent : public UInputHandlerComponentBase
{
	GENERATED_BODY()

	// Input.

protected:

	/** Binds this component's handlers. */
	virtual void BindInputHandlers(UCrashInputComponent* CrashIC, const UCrashInputActionMapping* ActionMapping) override;

	/** Base right/left turn rate for gamepad look input. */
	UPROPERTY(EditDefaultsOnly, Category = Input, DisplayName = "Base Turn Rate: Yaw")
	float BaseLookRate_Yaw = 165.0f;

	/** Base up/down turn rate for gamepad look input. */
	UPROPERTY(EditDefaultsOnly, Category = Input, DisplayName = "Base Turn Rate: Pitch")
	float BaseLookRate_Pitch = 165.0f;



	// Handler functions.

protected:

	/** Look input using a mouse. */
	void Input_Look_Mouse(const FInputActionValue& InputActionValue);

	/** Look input using a gamepad stick. */
	void Input_Look_Stick(const FInputActionValue& InputActionValue);

	/** Simple 4D movement input. */
	void Input_Move(const FInputActionValue& InputActionValue);
};
