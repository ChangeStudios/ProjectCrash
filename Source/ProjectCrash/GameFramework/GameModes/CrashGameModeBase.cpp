// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/CrashGameModeBase.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGlobalAbilitySystem.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

void ACrashGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Add the Death ability to the global ability system, which will grant it to each ASC as they are created.
	if (UCrashGlobalAbilitySystem* GlobalAbilitySystem = UWorld::GetSubsystem<UCrashGlobalAbilitySystem>(GetWorld()))
	{
		if (DefaultDeathAbility)
		{
			GlobalAbilitySystem->GrantGlobalAbility(DefaultDeathAbility);
		}
		else
		{
			ABILITY_LOG(Fatal, TEXT("ACrashGameModeBase: Game Mode [%s] does not have a default Death ability. Death logic will not function properly."), *GetName());
		}
	}
}

void ACrashGameModeBase::StartDeath(AActor* DyingActor)
{
	check(DyingActor);

	// Cache the player controlling the dying actor, if it's a player-controlled pawn.
	APawn* Pawn = Cast<APawn>(DyingActor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	UPlayer* Player = PC ? PC->Player : nullptr;
	const bool bPlayerDeath = IsValid(Player);

	/* Activate the Death ability on the dying actor, if they have an ASC. Note: Dying actors should always have an ASC,
	 * since deaths are triggered by the Health attribute set, which requires an ASC. */
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(DyingActor))
	{
		const FGameplayAbilitySpec* AbilitySpec = CrashASC->FindAbilitySpecFromClass(DefaultDeathAbility);
		CrashASC->TryActivateAbility(AbilitySpec->Handle);
	}

	UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] died. Executing [%s] death."), *DyingActor->GetName(), *FString(bPlayerDeath ? "PLAYER PAWN" : "NON-PLAYER ACTOR"));
}

void ACrashGameModeBase::FinishDeath(AActor* DyingActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Death ended"));
}
