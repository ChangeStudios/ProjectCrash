// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalGameData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTCRASH_API UGlobalGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Teams.

// Team fresnel materials.
public:

	/** Fresnel to use on "friendly" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Friendly Fresnel")
	UMaterialInstance* TeamFresnel_Friendly;

	/** Fresnel to use on "neutral" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Neutral Fresnel")
	UMaterialInstance* TeamFresnel_Neutral;

	/** Generic fresnel to use on hostile actors, if each hostile team should get the same fresnel. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Hostile Fresnel")
	UMaterialInstance* TeamFresnel_Hostile;

	/** Fresnels to use on hostile actors, if each hostile team should get a unique fresnel. Used for team-based game-
	 * modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Hostile Team Fresnels")
	TArray<UMaterialInstance*> HostileTeamFresnels;
};
