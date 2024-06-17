// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "Components/PawnComponent.h"
#include "InputHandlerComponentBase.generated.h"

/**
 * Defines input for pawns. Applies input configurations and defines handlers for gameplay abilities. This component
 * should be subclassed to define pawn-specific input handlers.
 *
 * This component requires that its owning pawn has a pawn extension component for proper initialization.
 */
UCLASS(Blueprintable)
class PROJECTCRASH_API UInputHandlerComponentBase : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()


};
