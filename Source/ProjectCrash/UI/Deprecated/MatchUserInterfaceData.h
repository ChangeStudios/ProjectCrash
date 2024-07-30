// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Deprecated/UserInterfaceData.h"
#include "MatchUserInterfaceData.generated.h"

/**
 * Extension of the base UI data that defines additional properties specific to gameplay, such as a pause menu.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCRASH_API UMatchUserInterfaceData : public UUserInterfaceData
{
	GENERATED_BODY()

	// End-game

public:

	UPROPERTY(EditDefaultsOnly, Category = "Match|End-Game")
	TSubclassOf<UCommonActivatableWidget> VictoryPopUp;

	UPROPERTY(EditDefaultsOnly, Category = "Match|End-Game")
	TSubclassOf<UCommonActivatableWidget> DefeatPopUp;
};
