// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameMode.h"

#include "AbilitySystemLog.h"
#include "CrashGameModeData.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Player/PlayerStates/CrashPlayerState.h"

void ACrashGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Add the Death ability to the global ability system, which will grant it to each ASC as they are created.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		if (GameModeData->DefaultDeathAbility)
		{
			GlobalAbilitySystem->GrantGlobalAbility(GameModeData->DefaultDeathAbility);
		}
		else
		{
			ABILITY_LOG(Fatal, TEXT("ACrashGameModeBase: Game Mode [%s] does not have a default Death ability. Death logic will not function properly."), *GetName());
		}
	}
}

void ACrashGameMode::StartDeath(AActor* DyingActor)
{
	check(DyingActor);

	// Cache the player controlling the dying actor, if it's a player-controlled pawn.
	APawn* Pawn = Cast<APawn>(DyingActor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	UPlayer* Player = PC ? PC->Player : nullptr;
	const bool bPlayerDeath = IsValid(Player);

	UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] died. Executing [%s] death."), *DyingActor->GetName(), *FString(bPlayerDeath ? "PLAYER PAWN" : "NON-PLAYER ACTOR"));

	/* Activate the Death ability on the dying actor, if they have an ASC. Note: Dying actors should always have an ASC,
	 * since deaths are triggered by the Health attribute set, which requires an ASC. */
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(DyingActor))
	{
		const FGameplayAbilitySpec* AbilitySpec = CrashASC->FindAbilitySpecFromClass(GameModeData->DefaultDeathAbility);
		const bool bDeathAbilitySuccess = CrashASC->TryActivateAbility(AbilitySpec->Handle);

		// Start a timer to finish the Death after DeathDuration.
		GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, FTimerDelegate::CreateLambda([this, DyingActor, CrashASC, bDeathAbilitySuccess, AbilitySpec]
		{
			// Only pass in the Death ability's handle if it was successfully activated.
			FinishDeath(DyingActor, CrashASC, bDeathAbilitySuccess ? AbilitySpec : nullptr);
		}), GameModeData->DeathDuration, false);
	}

	/* If a player died, decrement their lives. The player state will handle the rest, and notify us if the player is
	 * now out of lives. */
	if (bPlayerDeath)
	{
		if (ACrashPlayerState* CrashPS = PC->GetPlayerState<ACrashPlayerState>())
		{
			CrashPS->DecrementLives();
		}
	}
}

void ACrashGameMode::FinishDeath(AActor* DyingActor, UCrashAbilitySystemComponent* CrashASC, const FGameplayAbilitySpec* DeathAbility)
{
	// If we activated the Death ability, end it when the death finishes.
    if (DeathAbility != nullptr)
    {
    	CrashASC->CancelAbilityHandle(DeathAbility->Handle); // EndAbility is protected so we just have to cancel it (which does the same thing).
    }

	// Handle respawn if the game is not over.

    UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] successfully died. Finishing death..."), *GetNameSafe(DyingActor));
}