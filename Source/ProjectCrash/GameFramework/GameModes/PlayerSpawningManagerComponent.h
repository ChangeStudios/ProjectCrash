// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameStateComponent.h"
#include "PlayerSpawningManagerComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTCRASH_API UPlayerSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
    UPlayerSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);



	// Initialization.

public:

	/** Collects every player start in the level to use when determining where to spawn players. */
	virtual void InitializeComponent() override;

	/** Collects any new player starts for determining where to spawn players. */
	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);



	// Spawning.

protected:

	// TODO
};
