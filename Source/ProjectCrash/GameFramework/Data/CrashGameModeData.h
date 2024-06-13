// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashGameModeData.generated.h"

class UChallengerData;
class UGameFeatureAction;
class UGameFeatureActionSet;

/**
 * Static data defining a specific game mode (FFA TDM, team TDM, CTF, etc.). The data contained here is only relevant
 * DURING a game or pre-game (i.e. character selection) of this type. I.e. this should never be loaded in menus.
 *
 * The game state is responsible for retrieving this object from the game options, replicating it, and loading it.
 */
UCLASS(BlueprintType, NotBlueprintable, Const, DisplayName = "Game Mode Data")
class PROJECTCRASH_API UCrashGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Challenger selection.

public:

	/** If true, Challengers will NEVER be spawned for players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode", DisplayName = "Is Front End?")
	bool bIsFrontEnd = false;

	/** If enabled, players will be restricted to using the Challengers defined by this game mode when selecting their
	 * Challenger. If false, all core Challengers will be listed (Challengers with CoreChallenger enabled in their
	 * defining data). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", DisplayName = "Restrict Challengers?", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	bool bRestrictChallengers = false;

	/** If true, Challengers will be categorized in the character selection screen by their class. Useful for game
	 * modes where the available Challengers are restricted to generic playable characters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", DisplayName = "Categorize Challengers?", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	bool bCategorizeChallengers = true;

	/** The Challengers available for players to use in this game mode, if RestrictChallengers is true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Selection", Meta = (AllowedTypes = "ChallengerData", EditCondition = "bRestrictChallengers && !bIsFrontEnd", EditConditionHides = "true"))
	TArray<FPrimaryAssetId> AvailableChallengers;



	// Metadata.

public:

	/** The title of this game mode; e.g. "Deathmatch." Used when telling players what game mode they are currently
	 * playing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Title", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	FText UserFacingTitle;

	/** An optional subtitle describing this game mode; e.g. "Free-For-All." Paired with the user-facing title to
	 * describe this exact game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Subtitle", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	FText UserFacingSubTitle;



	// Teams.

public:

	/** The desired number of teams created in this game mode. In matchmaking, this is the required number of teams to
	 * start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Number of Teams", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	uint8 NumTeams = 0;

	/** The desired number of players on each teamm in this game mode. In matchmaking, this is the number of players
	 * required on each team to start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Number of Players Per Team", Meta = (EditCondition = "!bIsFrontEnd", EditConditionHides = "true"))
	uint8 PlayersPerTeam = 0;



	// Game features.

public:

	/** Actions to perform as this game mode is loaded, activated, deactivated, and unloaded. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "Game Features")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	/** A collection of game features that will be used in addition to Actions. Helpful for reusing common collections
	 * of game actions. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Features")
	TArray<TObjectPtr<UGameFeatureActionSet>> ActionSets;

	/** Game feature plugins that need to be loaded for this game mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Features")
	TArray<FString> GameFeaturesToEnable;



	// Validation.

public:

#if WITH_EDITOR
	/** Validates the data in this game mode's game feature actions. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR



	// Asset management.

public:

#if WITH_EDITORONLY_DATA
	/** Adds any extra asset bundle data needed by this game mode's game feature actions. */
	virtual void UpdateAssetBundleData() override;
#endif // WITH_EDITORONLY_DATA
};
