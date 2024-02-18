// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GA_Death.generated.h"

/**
 * Called on dying characters to handle their death. Cancels ongoing abilities, calls "StartDeath" in the gamemode and
 * calls "FinishDeath" in the gamemode after a delay.
 */
UCLASS()
class PROJECTCRASH_API UGA_Death : public UCrashGameplayAbilityBase
{
	GENERATED_BODY()

	// Ability logic.

public:

	/** Default constructor. */
	UGA_Death(const FObjectInitializer& ObjectInitializer);

	/** Cancels ongoing abilities, calls "StartDeath" on the gamemode, and sets a timer to finish  this ability. */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/** Calls "FinishDeath" on the gamemode. */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	/** A reference to the actor that is dying, since we will not be able to retrieve it after the owner abandons it. */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Data")
	AActor* DyingActor;
};