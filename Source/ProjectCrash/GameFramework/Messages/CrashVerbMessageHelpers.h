// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrashVerbMessageHelpers.generated.h"

struct FCrashVerbMessage;
struct FGameplayCueParameters;

class APlayerController;
class APlayerState;
class UObject;

/**
 * Utilities for reading and manipulating verb messages.
 */
UCLASS()
class PROJECTCRASH_API UCrashVerbMessageHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Attempts to retrieve any player state associated with the given object. */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	/** Attempts to retrieve any player controller associated with the given object. */
	UFUNCTION(BlueprintCallable, Category = "Messages")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);
};
