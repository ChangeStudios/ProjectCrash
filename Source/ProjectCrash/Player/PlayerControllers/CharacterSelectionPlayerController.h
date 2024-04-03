// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "CharacterSelectionPlayerController.generated.h"

/**
 * The player controller used in the character selection pre-game menu.
 */
UCLASS()
class PROJECTCRASH_API ACharacterSelectionPlayerController : public ACrashPlayerController
{
	GENERATED_BODY()

public:

	/** Initializes UI-only input. */
	virtual void BeginPlay() override;

protected:

	/** The UI data to create the character selection UI. */
	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TObjectPtr<UUserInterfaceData> CharacterSelectionUIData;
};
