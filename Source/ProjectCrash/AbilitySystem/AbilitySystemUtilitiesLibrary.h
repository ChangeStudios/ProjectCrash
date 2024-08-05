// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystemUtilitiesLibrary.generated.h"

/**
 * Collection of utilities for scripting gameplay abilities.
 */
UCLASS(Meta = (BlueprintThreadSafe, ScriptName = "AbilitySystemUtilitiesLibrary"), MinimalAPI)
class UAbilitySystemUtilitiesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
