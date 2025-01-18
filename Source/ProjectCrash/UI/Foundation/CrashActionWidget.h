// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActionWidget.h"
#include "GameplayTagContainer.h"
#include "CrashActionWidget.generated.h"

class UCommonTextBlock;
class UEnhancedInputLocalPlayerSubsystem;
class UImage;

/**
 * A widget that displays the icon or name of the key bound to the specified enhanced input action. If the action has a
 * predefined brush (defined by "Controller Data" classes, usually used for gamepad and mouse inputs), that brush will
 * be displayed.
 *
 * If a brush does not exist, a dynamic icon can be created by binding this widget to a text block and an image. When
 * a key has no predefined brush, the text block will be set to the key's name, the image will be made visible, and this
 * widget will be hidden.
 *
 * It's recommended to create a binding to the enhanced input action variable, so subclasses (e.g. the widgets for
 * different abilities) can set which action to use.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	/** Determines what brush to display for this widget. If a brush has been defined for the current enhanced input
	 * action, that brush will be displayed. Otherwise, this widget will be hidden, and the dynamic key widgets will be
	 * displayed instead. */
	virtual FSlateBrush GetIcon() const override;

	/** When there is no defined brush for this widget's input action's key, this widget's text will be set to the
	 * key's display name to create a dynamic key icon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	TObjectPtr<UCommonTextBlock> DynamicKeyIconDisplayName;

	/**
	 * When there is no defined brush for this widget's input action's key, this widget will be made visible to serve
	 * as a dynamic background for the key's name, which will be used to create a dynamic key icon instead. Note that
	 * this will be hidden in-game when necessary, but will not appear hidden in the editor, since UMG always draws
	 * widgets during design time regardless of their visibility.
	 *
	 * NOTE: The reason we're using a separate widget for dynamic key icons is because the alignment of predefined key
	 * brushes needs to be different from that of dynamic key icons: brushes need to be centered, but dynamic icons need
	 * to be filled, so they adapt to the length of the key name. It's (slightly) less annoying to just use two
	 * different widgets than trying to change this widget's alignment during runtime.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget")
	TObjectPtr<UImage> DynamicKeyIconBackground;

#if WITH_EDITORONLY_DATA
	/** Set this to preview this widget with a predefined key brush, since they can't be retrieved during design time
	 * (there's no enhanced input subsystem). If a design-time key is set, that will be used instead. Note that
	 * EnhancedInputAction has to be set to see this in the editor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CommonActionWidget", Meta = (EditCondition = "DesignTimeKey == NAME_None"))
	FSlateBrush DesignTimeBrush;
#endif

protected:

	/** Updates this widget's displayed icon and the dynamic key icon. */
	virtual void UpdateActionWidget() override;

	/** Toggles visibility for and updates a dynamic key icon for this widget's input action's key depending on whether
	 * there is a brush defined for it. */
	virtual void UpdateDynamicKeyIcon();

private:

	/** Retrieves the display name of the key to which this widget's input action is bound. */
	FText GetKeyDisplayName() const;

	/** Retrieves this widget's owning player's enhanced input system. */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};
