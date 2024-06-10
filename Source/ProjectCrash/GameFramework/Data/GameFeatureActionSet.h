// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFeatureActionSet.generated.h"

class UGameFeatureAction;

/**
 * A collection of game feature actions used in modularly defining game modes. Helpful for avoiding having to re-enter
 * common data; e.g. having to add a HUD to every standard game mode.
 */
UCLASS(BlueprintType, NotBlueprintable)
class PROJECTCRASH_API UGameFeatureActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Game features.

public:

	/** Actions to perform as this game mode is loaded, activated, deactivated, and unloaded. */
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	/** Game feature plugins that need to be loaded for this set's actions. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Features")
	TArray<FString> GameFeaturesToEnable;



	// Validation.

public:

#if WITH_EDITOR
	/** Validates the data in this set's game feature actions. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR



	// Asset management.

public:

#if WITH_EDITORONLY_DATA
	/** Adds any extra asset bundle data needed by this set's game feature actions. */
	virtual void UpdateAssetBundleData() override;
#endif // WITH_EDITORONLY_DATA
};
