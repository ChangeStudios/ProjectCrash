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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	UMaterialInstance* TeamFresnel_Friendly;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	UMaterialInstance* TeamFresnel_Hostile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams")
	UMaterialInstance* TeamFresnel_Neutral;
};
