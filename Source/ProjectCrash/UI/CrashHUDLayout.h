// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CrashActivatableWidget.h"
#include "CrashHUDLayout.generated.h"

/**
 * A widget that defines the layout of an in-game HUD using extension points. Widgets can be inserted into extension
 * points, either via the "Add Widgets" game feature action or the "Register Extension" functions in the UI Extension
 * Subsystem.
 *
 * HUD layouts should rarely contain actual widgets and should never implement any logic.
 *
 * This class also provides an escape menu (e.g. an in-game pause menu) using an "Escape" Common UI action.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTCRASH_API UCrashHUDLayout : public UCrashActivatableWidget
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UCrashHUDLayout(const FObjectInitializer& ObjectInitializer);

	/** Registers the "escape" action to push the escape menu to the "Menu" layer. */
	virtual void NativeOnInitialized() override;

protected:

	/** Pushes the specified escape menu widget to the local player. */
	void HandleEscapeAction();

	/** The widget to push to the owning player when the "escape" action is triggered. */
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
};
