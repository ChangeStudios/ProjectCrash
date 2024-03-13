// Copyright Samuel Reitich 2024.

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



	// Physics detection.

public:

	/** Grant this volume's abilities and apply its effects to any actors with an ASC when they enter the volume. */
	virtual void ActorEnteredVolume(AActor* Other) override;

	/** Remove this volume's temporary abilities and effects from actors leaving the volume if they were given by this
	 * volume. */
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
	 * Note that these effects' handles are not saved, and these effects will not be automatically removed by this volume.
	 * As such, they should either be fire-and-forget effects, duration-based, or manually removed by class.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Effects", EditAnywhere)
	TArray<TSubclassOf<UGameplayEffect>> OnExitEffectsToApply;

// Abilities.
protected:

	/** Gameplay abilities given to an actor when it enters this volume. These will be removed when the actors exits
	 * this volume. */
	UPROPERTY(BlueprintReadWrite, Category = "Ability System|Gameplay Abilities", EditAnywhere)
	TArray<TSubclassOf<UGameplayAbility>> OngoingAbilitiesToGive;

	/**
	 * Gameplay abilities given to an actor when they enter this volume.
	 *
	 * Note that these abilities' handles are not saved, and these abilities will not be automatically removed by this
	 * volume. If they should ever been removed, they must be manually removed by the ASC by class.
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

	/** Actors currently inside this physics volume, mapped to a handle containing their temporarily granted abilities
	 * and applied effects that will be removed when the actor leaves. */
	UPROPERTY()
	TMap<AActor*, FEnteredActorAbilitySystemHandle> EnteredActorHandles;



	// Debug.

public:

	// Draws debug info when ability system debugging is enabled.
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
