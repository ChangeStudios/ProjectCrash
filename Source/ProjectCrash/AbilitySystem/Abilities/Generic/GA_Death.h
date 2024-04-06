// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CrashGameplayAbilityBase.h"
#include "GA_Death.generated.h"

/**
 * Data used when performing actor death logic. Most of this information is given by the Health attribute set upon
 * taking lethal damage, and automatically passed to the Death gameplay ability when it's activated via gameplay event.
 */
USTRUCT(BlueprintType)
struct FDeathData
{
	GENERATED_BODY()

	/** Default constructor. Initializes all values to null. */
	FDeathData()
		: DyingActor(nullptr), DyingPlayer(nullptr), DyingActorASC(nullptr), DamageInstigator(nullptr), KillingDamageCauser(nullptr), DamageEffectSpec(FGameplayEffectSpec()), DamageMagnitude(0.0f)
	{}

	/** Parameterized constructor. */
	FDeathData(AActor* InDyingActor, APlayerController* InDyingPlayer, UAbilitySystemComponent* InDyingActorASC, AActor* InDamageInstigator, AActor* InKillingDamageCauser, const FGameplayEffectSpec& InDamageEffectSpec, float InDamageMagnitude) :
		DyingActor(InDyingActor),
		DyingPlayer(InDyingPlayer),
		DyingActorASC(InDyingActorASC),
		DamageInstigator(InDamageInstigator),
		KillingDamageCauser(InKillingDamageCauser),
		DamageEffectSpec(InDamageEffectSpec),
		DamageMagnitude(InDamageMagnitude)
	{}

	/** Actor that died. This is usually an ASC's avatar; e.g. a player pawn. */
	UPROPERTY(BlueprintReadOnly)
	AActor* DyingActor;

	/** The player controlling the dying actor. Will be null for dying non-player actors. */
	UPROPERTY(BlueprintReadOnly)
	APlayerController* DyingPlayer;

	/** ASC associated with the dying actor. The dying actor is usually the avatar or/and the owner of this ASC. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Dying Actor's ASC")
	UAbilitySystemComponent* DyingActorASC;

	/** Actor responsible for instigating the damage that killed this actor; i.e. the owner of the damaging ASC. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Killing Damage Instigator")
	AActor* DamageInstigator;

	/** Actor that directly caused the damage that killed this actor; e.g. an enemy player pawn or a grenade. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Killing Damage Causer")
	AActor* KillingDamageCauser;

	/** Gameplay effect that applied the damage that killed this actor. */
	FGameplayEffectSpec DamageEffectSpec;

	/** Magnitude of the damage that killed this actor. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Killing Damage Magnitude")
	float DamageMagnitude;
};



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

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};