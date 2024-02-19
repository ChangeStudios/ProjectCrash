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

void ACrashGameMode::StartDeath(const FDeathData& DeathData)
{
	/* Send a gameplay event to the ASC of the dying actor to trigger the Death gameplay ability, which handles
	 * client-side death logic. */
	if (DeathData.DyingActorASC)
	{
		{
			FGameplayEventData Payload;
			Payload.EventTag = CrashGameplayTags::TAG_Event_Death;
			Payload.Instigator = DeathData.DamageInstigator;
			Payload.Target = DeathData.DyingActorASC->GetAvatarActor();
			Payload.OptionalObject = DeathData.DamageEffectSpec.Def;
			Payload.OptionalObject2 = DeathData.KillingDamageCauser;
			Payload.ContextHandle = DeathData.DamageEffectSpec.GetEffectContext();
			Payload.InstigatorTags = *DeathData.DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DeathData.DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();

			FScopedPredictionWindow NewScopedWindow(DeathData.DyingActorASC, true);
			DeathData.DyingActorASC->HandleGameplayEvent(Payload.EventTag, &Payload);
		}
	}

	// Start a timer to finish the Death after DeathDuration.
	{
		GetWorld()->GetTimerManager().SetTimer(DeathTimerHandle, FTimerDelegate::CreateLambda([this, DeathData]
		{
			FinishDeath(DeathData);
		}), GameModeData->DeathDuration, false);
	}

	// Cache the player controlling the dying actor, if it's a player-controlled pawn.
	APawn* Pawn = Cast<APawn>(DeathData.DyingActor);
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	UPlayer* Player = PC ? PC->Player : nullptr;
	const bool bPlayerDeath = IsValid(Player);

	UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] died. Executing [%s] death."), *DeathData.DyingActor->GetName(), *FString(bPlayerDeath ? "PLAYER PAWN" : "NON-PLAYER ACTOR"));

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

void ACrashGameMode::FinishDeath(const FDeathData& DeathData)
{
	// End the Death ability when the death finishes.
	if (DeathData.DyingActorASC)
	{
		const FGameplayTagContainer DeathTags = FGameplayTagContainer(CrashGameplayTags::TAG_Event_Death);
		DeathData.DyingActorASC->CancelAbilities(&DeathTags);
	}

	// Handle respawn if the game is not over.

    UE_LOG(LogGameMode, Verbose, TEXT("ACrashGameModeBase: Actor [%s] successfully died. Finishing death..."), *GetNameSafe(DeathData.DyingActor));
}
