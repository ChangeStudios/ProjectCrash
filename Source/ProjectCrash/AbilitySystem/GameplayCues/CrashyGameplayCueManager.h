// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "CrashyGameplayCueManager.generated.h"

/**
 * This project's default gameplay cue manager.
 */
UCLASS()
class PROJECTCRASH_API UCrashyGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

protected:

	/** Prevents the game from loading every single gameplay cue into memory when the game starts. Instead, they'll be
	 * asynchronously loaded when they're activated for the first time. */
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	
};
