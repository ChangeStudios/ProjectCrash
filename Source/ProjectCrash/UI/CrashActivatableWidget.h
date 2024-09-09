// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CrashActivatableWidget.generated.h"

class ACrashPlayerController;
struct FUIInputConfig;

/**
 * Input configurations with which widgets can be activated.
 */
UENUM(BlueprintType)
enum class ECrashWidgetInputMode : uint8
{
	// Input is received by game and UI.
	GameAndMenu,
	// Input is only received by game.
	Game,
	// Input is only received by UI; mouse is never captured (mouse capture settings are ignored).
	Menu
};



/**
 * Base activatable widget class for this project. Provides input configuration options and various utilities. Primarily
 * intended for menus; game HUDs should use user widgets.
 */
UCLASS(Abstract)
class PROJECTCRASH_API UCrashActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashActivatableWidget(const FObjectInitializer& ObjectInitializer);

	/** Plays an optional activation animation. */
	virtual void NativeOnActivated() override;



	// Input.

public:

	/** Uses this widget's input properties to determine the desired input configuration when this widget is
	 * activated. */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:

	/** The desired input mode to use while this UI is activated. Determines whether input is routed to the game, user
	 * interface, or both. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	ECrashWidgetInputMode InputConfig = ECrashWidgetInputMode::Menu;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;



	// Animations.

protected:

	/** Optional animation played when this widget is activated. To use this, simply create a widget animation named
	 * "OnActivated." */
	UPROPERTY(Transient, BlueprintReadOnly, Meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OnActivated;



	// Utils.

public:

	/** Returns this widget's owning player controller as a CrashPlayerController. Returns null if the player is
	 * invalid or is not of the correct type. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Widget")
	ACrashPlayerController* GetOwningCrashPlayer() const;



	// Validation.

public:

#if WITH_EDITOR

	/** Checks if the "GetDesiredFocusTarget" function is implemented on this widget or one of its parents. Without
	 * this function, gamepads will not be able to use menu-based widgets. */
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const override;

#endif // WITH_EDITOR
};
