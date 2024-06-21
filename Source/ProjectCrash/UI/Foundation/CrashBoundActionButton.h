// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/CommonBoundActionButton.h"
#include "CrashBoundActionButton.generated.h"

/**
 * A button that automatically switches between predefined styles depending on its owner's current input method. Also
 * provides utilities for implementing custom input method logic.
 */
UCLASS(Abstract, Meta = (DisableNativeTick))
class PROJECTCRASH_API UCrashBoundActionButton : public UCommonBoundActionButton
{
	GENERATED_BODY()

	// Construction.

protected:

	/** Binds callbacks to when this widget's owning player's input method changes. */
	virtual void NativeConstruct() override;

	/** Unbinds this widget's callbacks. */
	virtual void NativeDestruct() override;



	// Input method handling.

protected:

	/** Called when the owning player's input method changes. */
	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInputMethodChanged")
	void K2_OnInputMethodChanged(ECommonInputType NewInputMethod);

private:

	/** Switches to the new input method's style and calls K2_OnInputMethodChanged when the owning player's input
	 * method changes. */
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);



// Input method styles.
private:

	/** Optional style to use for this widget when the user is in an MnK input method. */
	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> KeyboardStyle;

	/** Optional style to use for this widget when the user is in a gamepad input method.*/
	UPROPERTY(EditAnywhere, Category = "Styles")
	TSubclassOf<UCommonButtonStyle> GamepadStyle;
};
