// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CrashActivatableWidget.generated.h"

class ACrashPlayerController;

/**
 * Base activatable widget class for this project. Provides various utilities for convenience.
 */
UCLASS()
class PROJECTCRASH_API UCrashActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	// Utils.

public:

	/** Returns this widget's owning player controller cast to ACrashPlayerController. Returns null if the player is
	 * invalid OR is not of the correct class. */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Widget")
	FORCEINLINE ACrashPlayerController* GetOwningCrashPlayer() const;
};
