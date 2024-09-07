// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalGameData.generated.h"

class UGameplayEffect;

/**
 * Global game data synchronously loaded by the asset manager when the game starts. This data is ALWAYS globally
 * accessible via the asset manager, and should be kept as small as possible.
 */
UCLASS()
class PROJECTCRASH_API UGlobalGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UGlobalGameData();

	/** Retrieves the loaded game data. */
	static const UGlobalGameData& Get();



	// Gameplay effects.

public:

	/** Gameplay effect used to apply damage from code. Uses set-by-caller for magnitude. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", DisplayName = "Damage Gameplay Effect (Set-By-Caller)")
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;
};
