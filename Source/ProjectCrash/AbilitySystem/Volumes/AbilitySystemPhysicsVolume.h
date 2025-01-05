// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PhysicsVolume.h"
#include "AbilitySystemPhysicsVolume.generated.h"

class UGameplayAbility;
class UGameplayEffect;

/** Handle for actors currently inside the ability system physics volume. Holds data for abilities and effects that
 * need to be removed when the actor leaves. */
USTRUCT()
struct FEnteredActorAbilitySystemHandle
{
	GENERATED_BODY()

	/** Granted abilities that will be removed when the actor leaves. */
	TArray<FGameplayAbilitySpecHandle> TemporarilyGrantedAbilities;

	/** Applied effects that will be removed when the actor leaves. */
	TArray<FActiveGameplayEffectHandle> TemporarilyAppliedEffects;
};



/**
 * Physics volume that can grant abilities, apply gameplay effects, and fire gameplay events when an actor with an ASC
 * enters and/or exits the volume.
 */
UCLASS()
class PROJECTCRASH_API AAbilitySystemPhysicsVolume : public APhysicsVolume
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	AAbilitySystemPhysicsVolume();



	// Initialization.

public:

	/** Binds debug info to the ability system debugger. */
	virtual void BeginPlay() override;



	// Volume events.

public:

	/** Grants this volume's abilities, applies its effects, and fires its gameplay events to any actors with an ASC
	 * when they enter the volume. */
	virtual void ActorEnteredVolume(AActor* Other) override;

	/** Removes this volume's temporary abilities and effects from actors leaving the volume. Fires exiting gameplay
	 * events on any actors with an ASC. */
	virtual void ActorLeavingVolume(AActor* Other) override;



	// Ability system.

// Effects.
protected:

	/** Gameplay effects to apply to an actor when it enters this volume. These will be removed when the actor exits
	 * this volume. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Effects", EditAnywhere)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApply;

	/**
	 * Gameplay effects to apply to an actor when it leaves this volume.
	 *
	 * These effects' handles are not saved, and these effects will not be automatically removed by this volume. Unless
	 * they are permanent effects, they should either be instant effects, duration-based effects, or manually
	 * removed.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Effects", EditAnywhere)
	TArray<TSubclassOf<UGameplayEffect>> OnExitEffectsToApply;

// Abilities.
protected:

	/** Gameplay abilities given to an actor when it enters this volume. These will be removed when the actor exits
	 * this volume. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Abilities", EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> OngoingAbilitiesToGive;

	/**
	 * Gameplay abilities given to an actor when they enter this volume. These are not removed when the actor exists
	 * this volume.
	 *
	 * These abilities' handles are not saved. If they should ever been removed, they must be removed manually.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Abilities", EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> PermanentAbilitiesToGive;

// Events.
protected:

	/** Gameplay events sent to actors when they enter this volume. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Events", EditAnywhere, Meta = (Categories = "Event"))
	TArray<FGameplayTag> GameplayEventsToSendOnEnter;

	/** Gameplay events sent to actors when they exit this volume. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Events", EditAnywhere, Meta = (Categories = "Event"))
	TArray<FGameplayTag> GameplayEventsToSendOnExit;

// Internals.
protected:

	/** Actors currently inside this physics volume, mapped to a handle containing their granted abilities and applied
	 * effects that will be removed when the actor leaves. */
	UPROPERTY()
	TMap<AActor*, FEnteredActorAbilitySystemHandle> EnteredActorHandles;



	// Debugging.

public:

	// Draws debug info when ability system debugging is enabled.
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
