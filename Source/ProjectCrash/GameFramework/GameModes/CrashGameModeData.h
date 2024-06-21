// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashGameModeData.generated.h"

class UPawnData;
class UGameFeatureAction;
class UGameFeatureActionSet;

/**
 * Static data defining a game mode. Can be an in-game game mode or a menu game mode.
 */
UCLASS(BlueprintType, NotBlueprintable, Const, DisplayName = "Game Mode Data")
class PROJECTCRASH_API UCrashGameModeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UCrashGameModeData();



	// Game mode properties.

public:
	
	/** The default pawn for this game mode. In core game modes, where players select their characters beforehand, this
	 * should be null. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	TObjectPtr<UPawnData> DefaultPawn;

	/** Whether players should be immediately destroyed when they become inactive, or if they should be allowed to
	 * stay to be potentially re-activated if the player reconnects. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Mode")
	bool bDestroyDeactivatedPlayers;



	// Metadata.

public:

	/** The title of this game mode; e.g. "Deathmatch." Used when telling players what game mode they are currently
	 * playing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Title")
	FText UserFacingTitle;

	/** An optional subtitle describing this game mode; e.g. "Free-For-All." Paired with the user-facing title to
	 * describe this exact game mode. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Metadata", DisplayName = "User-Facing Subtitle")
	FText UserFacingSubTitle;



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
