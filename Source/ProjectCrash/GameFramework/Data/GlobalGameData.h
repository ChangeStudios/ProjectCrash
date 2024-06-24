// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalGameData.generated.h"

/**
 * Defines a collection of data used when identifying members of a team during gameplay.
 */
USTRUCT(BlueprintType)
struct FTeamColorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> TeamFresnel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor TeamUIColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
};



/**
 * Global game data synchronously loaded by the asset manager when the game starts. This data is ALWAYS globally
 * accessible via the asset manager, and should be kept as small as possible.
 */
UCLASS()
class PROJECTCRASH_API UGlobalGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Teams.

// Team fresnel materials.
public:

	/** Fresnel to use on "friendly" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Friendly Team Color", Meta = (DeprecatedProperty, DeprecatedMessage = "Team color data is outdated. Use the TeamCreationComponent instead."))
	FTeamColorData TeamColor_Friendly;

	/** Fresnel to use on "neutral" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Neutral Team Color", Meta = (DeprecatedProperty, DeprecatedMessage = "Team color data is outdated. Use the TeamCreationComponent instead."))
	FTeamColorData TeamColor_Neutral;

	/** Fresnels to use on hostile actors, if each hostile team should get a unique fresnel. Used for team-based game
	 * modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Hostile Team Colors", Meta = (DeprecatedProperty, DeprecatedMessage = "Team color data is outdated. Use the TeamCreationComponent instead."))
	TArray<FTeamColorData> TeamColor_HostileList;
};
