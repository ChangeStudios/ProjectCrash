// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CrashMathLibrary.generated.h"

/**
 * Provides various math-related utility functions for use in C++ and kismet.
 */
UCLASS(meta = (BlueprintThreadSafe), MinimalAPI)
class UCrashMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Generates a random vector with each value between its corresponding values in Min and Max. */
	UFUNCTION(BlueprintPure, Category="Math|Random")
	static PROJECTCRASH_API FVector RandomVectorInRange(const FVector& Min, const FVector& Max);
};
