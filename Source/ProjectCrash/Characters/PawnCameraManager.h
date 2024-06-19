// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "PawnCameraManager.generated.h"

/**
 * Sets up camera handling for pawns. The owning pawn should possess a CrashCameraComponent.
 *
 * This component requires that its owning pawn has a pawn extension component for proper initialization.
 */
UCLASS(NotBlueprintable, Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UPawnCameraManager : public UPawnComponent
{
	GENERATED_BODY()
	
	
	
	
};
