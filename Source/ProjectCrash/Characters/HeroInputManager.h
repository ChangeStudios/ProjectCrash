// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Characters/InputHandlerComponentBase.h"
#include "HeroInputManager.generated.h"

class UCrashInputActionMapping;

/**
 * Defines input for basic pawn movement.
 */
UCLASS(Meta = (BlueprintSpawnableComponent, ShortToolTip = "Sets up input for basic pawn movement. Requires a pawn extension component."))
class PROJECTCRASH_API UHeroInputManager : public UInputHandlerComponentBase
{
	GENERATED_BODY()
};
