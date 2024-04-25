// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CrashActivatableWidget.generated.h"

class ACrashPlayerControllerBase;
struct FUIInputConfig;

/**
 * The input modes with which widgets can behave, similar to viewports.
 */
UENUM(BlueprintType)
enum class ECrashWidgetInputMode : uint8
{
	Default,
	GameAndMenu,
	Game,
	Menu
};



/**
 * Base activatable widget class for this project. Provides various utilities for convenience.
 */
UCLASS()
class PROJECTCRASH_API UCrashActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashActivatableWidget(const FObjectInitializer& ObjectInitializer);

	/** Uses this widget's properties to determine its desired input configuration. */
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/** Plays an optional activation animation and sets up this widget's user focus. */
	virtual void NativeOnActivated() override;



	// Utils.

public:

	/** Returns this widget's owning player controller cast to ACrashPlayerController. Returns null if the player is
	 * invalid OR is not of the correct class. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Widget")
	ACrashPlayerControllerBase* GetOwningCrashPlayer() const;



	// Input.

protected:
	/** The desired input mode to use while this UI is activated. E.g. should key presses to still reach the player
	 * controller? */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	ECrashWidgetInputMode InputConfig = ECrashWidgetInputMode::Default;

	/** The desired mouse behavior when the game gets input. */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	EMouseCaptureMode GameMouseCaptureMode = EMouseCaptureMode::CapturePermanently;



	// Animations.

protected:

	/** Optional animation played when this widget is activated. */
	UPROPERTY(Transient, BlueprintReadOnly, Meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OnActivated;
};
