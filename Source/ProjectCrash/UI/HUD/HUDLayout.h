// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "UI/Framework/CrashActivatableWidget.h"
#include "HUDLayout.generated.h"

/**
 * A widget that defines the layout of an in-game HUD using slotted widgets.
 *
 * Also implements an escape menu using an "escape" Common UI action.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class PROJECTCRASH_API UHUDLayout : public UCrashActivatableWidget
{
	GENERATED_BODY()

public:

	// Registers the "escape" action to push the escape menu to the "Menu" layer.
	virtual void NativeOnInitialized() override;

protected:

	/** The widget to push to the owning player when the "escape" action is triggered. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommonActivatableWidget> EscapeMenuClass;
};
