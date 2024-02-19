// Copyright Samuel Reitich 2024.


#include "GameFramework/GameModes/Game/CrashGameMode.h"

#include "AbilitySystemLog.h"
#include "CrashGameModeData.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
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

void ACrashGameMode::StartDeath(AActor* DyingActor, UAbilitySystemComponent* DyingActorASC, AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec)
{
	/* Send a gameplay event to the ASC of the dying actor to trigger the Death gameplay ability, which handles
	 * client-side death logic. */
	if (DyingActorASC)
	{
		{
			FGameplayEventData Payload;
			Payload.EventTag = CrashGameplayTags::TAG_Event_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = DyingActorASC->GetAvatarActor();
			Payload.OptionalObject = DamageEffectSpec.Def;
			Payload.ContextHandle = DamageEffectSpec.GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();

			FScopedPredictionWindow NewScopedWindow(DyingActorASC, true);
			DyingActorASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}

	// Start a timer to finish the Death after DeathDuration.
	{
		GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, FTimerDelegate::CreateLambda([this, DyingActor, DyingActorASC]
		{
			FinishDeath(DyingActor, DyingActorASC);
		}), GameModeData->DeathDuration, false);
	}

	// Cache the player controlling the dying actor, if it's a player-controlled pawn.
	APawn* Pawn = Cast<APawn>(DyingActor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	UPlayer* Player = PC ? PC->Player : nullptr;
	const bool bPlayerDeath = IsValid(Player);

	UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] died. Executing [%s] death."), *DyingActor->GetName(), *FString(bPlayerDeath ? "PLAYER PAWN" : "NON-PLAYER ACTOR"));

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

void ACrashGameMode::FinishDeath(AActor* DyingActor, UAbilitySystemComponent* DyingActorASC)
{
	// End the Death ability when the death finishes.
	if (DyingActorASC)
	{
		const FGameplayTagContainer DeathTags = FGameplayTagContainer(CrashGameplayTags::TAG_Event_Death);
		DyingActorASC->CancelAbilities(&DeathTags);
	}

	// Handle respawn if the game is not over.

    UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] successfully died. Finishing death..."), *GetNameSafe(DyingActor));
}
