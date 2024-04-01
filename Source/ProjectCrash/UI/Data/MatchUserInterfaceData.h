// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "UI/Data/UserInterfaceData.h"
#include "MatchUserInterfaceData.generated.h"

/**
 * Extension of the base UI data that defines additional properties specific to gameplay, such as a pause menu.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTCRASH_API UMatchUserInterfaceData : public UUserInterfaceData
{
	GENERATED_BODY()

public:

	// TODO: Add a pause menu widget and an array of stackable widgets to put in it

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	TSubclassOf<UCommonActivatableWidget> CharacterSelectionWidget;
};
