// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalGameData.generated.h"

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
 * 
 */
UCLASS()
class PROJECTCRASH_API UGlobalGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Time.

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Time", DisplayName = "Post-Match Time Dilation")
	float EndMatchTimeDilation = 0.1f;



	// Teams.

// Team fresnel materials.
public:

	/** Fresnel to use on "friendly" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Friendly Team Color")
	FTeamColorData TeamColor_Friendly;

	/** Fresnel to use on "neutral" actors. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Neutral Team Color")
	FTeamColorData TeamColor_Neutral;

	/** Fresnels to use on hostile actors, if each hostile team should get a unique fresnel. Used for team-based game-
	 * modes. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teams", DisplayName = "Hostile Team Colors")
	TArray<FTeamColorData> TeamColor_HostileList;
};
