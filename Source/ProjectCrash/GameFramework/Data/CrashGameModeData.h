// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashGameModeData.generated.h"

class UUserInterfaceData;

/**
 * Static data defining a specific game mode (FFA TDM, team TDM, CTF, etc.). The data contained here is only relevant
 * DURING a game of this type. I.e. this should never be loaded in menus.
 *
 * The game state is responsible for retrieving this object from the game options, replicating it, and loading it.
 */
UCLASS(DisplayName = "Game Mode Data")
class PROJECTCRASH_API UCrashGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, DisplayName = "Maximum Time (Regulation)")
	float MaxTime_Regulation;

	UPROPERTY(EditDefaultsOnly, DisplayName = "User Interface Data")
	TObjectPtr<UUserInterfaceData> UIData;

};
