// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrashStatics.generated.h"

/**
 * Generic static utilities for this project.
 */
UCLASS()
class PROJECTCRASH_API UCrashStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Attempts to retrieve any player state associated with the given object. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Utils")
	static APlayerState* GetPlayerStateFromObject(UObject* Object);

	/** Attempts to retrieve any player controller associated with the given object. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Utils")
	static APlayerController* GetPlayerControllerFromObject(UObject* Object);

	/** Attempts to retrieve any controller associated with the given object. */
	UFUNCTION(BlueprintCallable, Category = "Crash|Utils")
	static AController* GetControllerFromObject(UObject* Object);
};
