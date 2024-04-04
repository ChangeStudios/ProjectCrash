// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashGameInstance.h"

#include "GameFramework/PlayerState.h"
#include "GameStates/CrashGameState_CharacterSelection.h"
#include "Kismet/GameplayStatics.h"

bool UCrashGameInstance::AssignPlayerChallenger(APlayerController* Player, const UChallengerData* SelectedChallenger)
{
	// Only update the players' Challengers on the server during the character selection phase.
	const AGameStateBase* GS = UGameplayStatics::GetGameState(Player);
	const ACrashGameState_CharacterSelection* CharSelectionGS = GS ? Cast<ACrashGameState_CharacterSelection>(GS) : nullptr;
	if (CharSelectionGS && CharSelectionGS->HasAuthority())
	{
		// Update the given player's Challenger.
		SelectedChallengers.Add(Player, SelectedChallenger);

		// Check if every player has a Challenger.
		for (const APlayerState* PS : GS->PlayerArray)
		{
			if (!SelectedChallengers.Contains(PS->GetPlayerController()))
			{
				return false;
			}
		}

		return true;
	}

	return false;
}
