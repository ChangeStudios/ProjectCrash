// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashGameModeData.generated.h"

class UGameFeatureActionSet;
class UGameFeatureAction;

/**
 * Static data defining a specific game mode (FFA TDM, team TDM, CTF, etc.). The data contained here is only relevant
 * DURING a game or pre-game (i.e. character selection) of this type. I.e. this should never be loaded in menus.
 *
 * The game state is responsible for retrieving this object from the game options, replicating it, and loading it.
 */
UCLASS(BlueprintType, Const, DisplayName = "Game Mode Data")
class PROJECTCRASH_API UCrashGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Challenger selection.

public:

	/** If enabled, players will be restricted to using the Challengers defined by this game mode when selecting their
	 * Challenger. If false, all core Challengers will be listed (Challengers with CoreChallenger enabled in their
	 * defining data). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", DisplayName = "Restrict Challengers?")
	bool bRestrictChallengers = false;

	/** If true, Challengers will be categorized in the character selection screen by their class. Useful for game
	 * modes where the available Challengers are restricted to generic playable characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", DisplayName = "Categorize Challengers?")
	bool bCategorizeChallengers = true;

	/** The Challengers available for players to use in this game mode, if RestrictChallengers is true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", Meta = (EditCondition = "bRestrictChallengers"))
	TArray<FPrimaryAssetId> AvailableChallengers;



	// Metadata.

public:

	/** The title of this game mode; e.g. "Deathmatch." Used when telling players what game mode they are currently
	 * playing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Title")
	FName UserFacingTitle;

	/** An optional subtitle describing this game mode; e.g. "Free-For-All." Paired with the user-facing title to
	 * describe this exact game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Subtitle")
	FName UserFacingSubTitle = NAME_None;



	// Game features.

public:

	// /** Game feature plugins that need to be loaded for this game mode. */
	// UPROPERTY(EditDefaultsOnly, Category = "Game Features")
	// TArray<FString> GameFeaturesToEnable;
	//
	// /** Actions to perform as this game mode is loaded, activated, deactivated, and unloaded. */
	// UPROPERTY(EditDefaultsOnly, Instanced, Category = "Game Features")
	// TArray<TObjectPtr<UGameFeatureAction>> Actions;
	//
	// /** A collection of game features that will be used in addition to Actions. Helpful for reusing common collections
	//  * of game actions. */
	// UPROPERTY(EditDefaultsOnly, Category = "Game Features")
	// TArray<TObjectPtr<UGameFeatureActionSet>> ActionSets;
};
