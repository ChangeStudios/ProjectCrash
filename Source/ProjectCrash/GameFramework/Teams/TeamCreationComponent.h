// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "TeamCreationComponent.generated.h"

class ACrashGameMode;
class ACrashPlayerState;
class UTeamDisplayAsset;
class UCrashGameModeData;

/**
 * Game state component responsible for creating teams when the game starts. Should be subclassed to define which teams
 * are created.
 *
 * Can be subclassed in C++ to change how teams are created. E.g. rather than having fixed teams, a game mode may want
 * to create a new team for each player when they join.
 */
UCLASS(Abstract, Blueprintable, HideCategories = (ComponentTick, Tags, ComponentReplication, Cooking, Activation, Variable, AssetUserData, Replication, Navigation), Meta = (ShortToolTip = "Defines which teams are created for a game."))
class PROJECTCRASH_API UTeamCreationComponent : public UGameStateComponent
{
	GENERATED_BODY()

	// Initialization.
	
public:

	/** Registers the creation of teams to when the game mode finishes loading. */
	virtual void BeginPlay() override;

private:

	/** Creates this component's specified teams when the game load loads. */
	void OnGameModeLoaded(const UCrashGameModeData* GameModeData);



	// Team creation.

#if WITH_SERVER_CODE

protected:

	/**
	 * Called after the game mode is loaded to create this component's teams.
	 *
	 * Default implementation simply creates a team for each team in the TeamsToCreate map.
	 */
	virtual void CreateTeams();

	/**
	 * Called after teams are created to assign players to teams.
	 *
	 * Default implementation assigns all existing players to a team with ChooseTeamForPlayer and starts listening for
	 * new players to assign them a team when they join.
	 */
	virtual void AssignPlayersToTeams();

	/**
	 * Selects the best team for the given player and assigns them to it.
	 *
	 * Default implementation assigns each player to the team with the fewest members. Assigns spectators to
	 * FGenericTeamId::NoTeam.
	 */
	virtual void ChooseTeamForPlayer(ACrashPlayerState* CrashPS);

private:

	/** Assigns new players a team when they join. */
	void OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer);

	/** Creates a new team with the given ID. The team will automatically register itself with the team subsystem. */
	void CreateTeam(int32 TeamId, UTeamDisplayAsset* DisplayAsset);

	/** Returns the team with the fewest members. */
	int32 GetLeastPopulatedTeam() const;

#endif // WITH_SERVER_CODE



	// Data.

protected:

	/** If true, teams will always use the same display asset when viewed by a team member. I.e. the player's team will
	 * always be the same color for them. */
	UPROPERTY(EditDefaultsOnly, Category = "Teams", DisplayName = "Use Friendly Display Asset?")
	bool bUseFriendlyDisplayAsset = true;

	/** If UseFriendlyDisplayAsset is true, this display asset will be used for teams when viewed by a team member. This
	 * asset must not be present in TeamsToCreate, or two teams may appear the same color for some players. */
	UPROPERTY(EditDefaultsOnly, Category = "Teams", Meta = (EditCondition = "bUseFriendlyDisplayAsset", EditConditionHides = "true"))
	TObjectPtr<UTeamDisplayAsset> FriendlyDisplayAsset = nullptr;

	/** The teams that will be created, represented by their team ID mapped to their corresponding display asset. */
	UPROPERTY(EditDefaultsOnly, Category = "Teams")
	TMap<uint8, TObjectPtr<UTeamDisplayAsset>> TeamsToCreate;



	// Validation.

public:

#if WITH_EDITOR
	/** Validates that every display asset has the same properties set. Ensures FriendlyDisplayAsset is not present in
	 * TeamsToCreate. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR

};
