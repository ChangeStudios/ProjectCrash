// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CrashButtonBase.generated.h"

/**
 * A button widget that supports 
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

	// Construction.

protected:

	/** Default constructor. */
	virtual void NativePreConstruct() override;



	// Properties.

// Property updating.
public:

	/** Sets this button's internal text value and refreshes the displayed text. */
	UFUNCTION(BlueprintCallable)
	void SetButtonText(const FText& InText);

protected:

	/** Updates this widget's displayed text with its current text value. */
	FORCEINLINE void RefreshButtonText();

	/** Called when this button's internal text value changes. Override this to update this button's displayed text
	 * with the new text value, given as a parameter. */
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonText(const FText& InText);

	/** Called to update this button's style with the value of its internal style properties. Override this to apply
	 * any internal style properties to this button's displayed style. */
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateButtonStyle();

// Internal properties.
private:

	/** Determines whether to use the value of the ButtonText property as this button's displayed text. If false, the
	 * displayed text will be hidden. */
	UPROPERTY(EditAnywhere, Category="Button Properties", Meta = (InlineEditConditionToggle))
	uint8 bOverride_ButtonText : 1;

	/** The internal property tracking the text that should be displayed by this button, if bOverride_ButtonText is
	 * true. */
	UPROPERTY(EditAnywhere, Category="Button Properties", Meta = (EditCondition = "bOverride_ButtonText" ))
	FText ButtonText;
	
};
