// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerControllers/CrashPlayerController_DEP.h"
#include "CharacterSelectionPlayerController.generated.h"

/**
 * The player controller used in the character selection pre-game menu.
 */
UCLASS()
class PROJECTCRASH_API ACharacterSelectionPlayerController : public ACrashPlayerController_DEP
{
	GENERATED_BODY()

public:

	/** Initializes UI-only input. */
	virtual void BeginPlay() override;

	/** Assigns the given player the specified Challenger. When all players in this game state have selected a
	 * Challenger, transitions to the game. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Player|Character Selection")
	void Server_LockInChallenger(const UChallengerData* SelectedChallenger);

protected:

	/** The UI data to create the character selection UI. */
	UPROPERTY(EditDefaultsOnly, Category = "User Interface")
	TObjectPtr<UUserInterfaceData> CharacterSelectionUIData;
};
