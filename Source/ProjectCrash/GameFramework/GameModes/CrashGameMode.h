// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameMode.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "CrashGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AModularGameModeBase, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashGameMode();
	
};
