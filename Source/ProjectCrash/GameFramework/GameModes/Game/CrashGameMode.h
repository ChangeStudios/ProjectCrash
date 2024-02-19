// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/GameMode.h"
#include "CrashGameMode.generated.h"

class UCrashGameModeData;
class UCrashAbilitySystemComponent;
class UGA_Death;

/**
 * The game mode used during gameplay (as opposed to menus, lobbies, etc.). Handles game setup, player death, and
 * victory conditions.
 */
UCLASS()
class PROJECTCRASH_API ACrashGameMode : public AGameMode
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Adds the Death gameplay ability to the global ability system, which will grant it to each ASC as they are
	 * created. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;



	// Game data.

public:

	/** Getter for GameModeData. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Game", meta = (ToolTip = "Data defining various properties of this game mode."))
	UCrashGameModeData* GetGameModeData() const { return GameModeData; }

protected:

	/** Data defining various properties of this game mode, such as players' default starting lives. */
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Data")
	TObjectPtr<UCrashGameModeData> GameModeData;



	// Death.

public:

	/**
	 * Handles the death (i.e. running out of health) of an actor depending on the game mode. Activates the Death
	 * gameplay ability to handle client-side death logic.
	 *
	 * @param	DyingActor			Actor that died. This is usually an ASC's avatar.
	 * @param	DyingActorASC		ASC associated with the dying actor. The dying actor is usually the avatar and/or the owner of the ASC.
	 * @param	DamageInstigator	Actor responsible for instigating the damage that killed this actor, e.g. an enemy player pawn.
	 * @param	DamageCauser		Actor that directly caused the damage that killed this actor, e.g. a grenade.
	 * @param	DamageEffectSpec	Gameplay effect that applied the damage that killed this actor.
	 */
	virtual void StartDeath(AActor* DyingActor, UAbilitySystemComponent* DyingActorASC, AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec& DamageEffectSpec);

protected:

	/** Ends the DeathAbility if one was given. */
	virtual void FinishDeath(AActor* DyingActor, UAbilitySystemComponent* DyingActorASC);

	/** Timer used to finish an actor death a certain amount of time after it was started. */
	FTimerHandle DeathTimerHandle;
};
