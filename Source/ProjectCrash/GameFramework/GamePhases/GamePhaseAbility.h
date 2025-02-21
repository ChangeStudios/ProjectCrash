// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GamePhaseAbility.generated.h"

/**
 * Base class for game phases. Represents an ongoing game phase while active. Game phases are identified by nested
 * tags: PreGame.CharacterSelection, Game.WarmUp, Game.Playing, etc.
 *
 * Game phase abilities only activate and run on the server and should communicate to clients via RPCs (e.g. gameplay
 * cues).
 *
 * @see UGamePhaseSubsystem
 */
UCLASS(Abstract, HideCategories = ("AbilityActivation", "UserInterface", "Cooldowns", Input, "Costs"))
class PROJECTCRASH_API UGamePhaseAbility : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

	// Initialization.
	
public:

	/** Default constructor. */
	UGamePhaseAbility(const FObjectInitializer& ObjectInitializer);



	// Ability.

protected:

	/** Notifies the game phase subsystem that this phase began. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Notifies the game phase subsystem that this phase ended. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;



	// Game phase.

public:

	/** Retrieves this game phase's tag. */
	const FGameplayTag& GetGamePhaseTag() const { return GamePhaseTag; }

protected:

	/** The identifying tag of the game phase that this ability represents. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Phase", Meta = (Categories = "GameMode.GamePhase"))
	FGameplayTag GamePhaseTag;



	// Manual phase controls.

public:

	/** Called when the subsystem's "StartMatch" function is called. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnMatchStarted();



	// Validation.

public:

#if WITH_EDITOR

	/** Ensures this ability's game phase tag is always set. */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;

#endif // WITH_EDITOR
};
