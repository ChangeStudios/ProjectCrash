// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "ActiveGameplayEffectHandle.h"
#include "CrashAbilitySet.generated.h"

class UAttributeSet;
class UCrashAbilitySystemComponent;
class UCrashAttributeSet;
class UCrashGameplayAbilityBase;
class UGameplayEffect;

/**
 * Data used to grant a gameplay ability via an ability set.
 */
USTRUCT(BlueprintType)
struct FCrashAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:

	/** The gameplay ability to grant. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCrashGameplayAbilityBase> GameplayAbility = nullptr;
};


/**
 * Data used to apply a gameplay effect via an ability set.
 */
USTRUCT(BlueprintType)
struct FCrashAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	/** The gameplay effect to grant. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	/** The level of the gameplay effect to grant. */
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};


/**
 * Data used to grant an attribute set via an ability set.
 */
USTRUCT(BlueprintType)
struct FCrashAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:

	/** The attribute set to grant. */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCrashAttributeSet> AttributeSet = nullptr;
};


/**
 * Collection of handles used to track and manage abilities, effects, and attribute sets granted by an ability set.
 */
USTRUCT(BlueprintType)
struct FCrashAbilitySet_GrantedHandles
{
	GENERATED_BODY()

	// Handle management.

public:

	/** Adds the given gameplay ability spec handle to the collection of ability handles. */
	void AddGameplayAbilitySpecHandle(const FGameplayAbilitySpecHandle& HandleToAdd);
	/** Adds the given gameplay effect handle to the collection of effect handles. */
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& HandleToAdd);
	/** Adds the given attribute set to the collection of tracked attribute sets. */
	void AddAttributeSet(UAttributeSet* SetToAdd);

	/** Removes every ability, effect, and attribute set in this structure from the specified ability system. */
	void RemoveFromAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToRemoveFrom);



	// Handle collections.

protected:

	/** Handles to the granted gameplay abilities. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;

	/** Handles to the applied gameplay effects. */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> AppliedEffectHandles;

	/** Pointers to the granted attribute sets. */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * A collection of abilities, effects, and attribute sets that are granted to and removed from an ASC together. This is
 * the *ONLY* way to grant abilities, as it performs crucial logic that links an input tag to an ability. Even if only
 * one ability is being granted, it must be granted via a set.
 *
 * This set-only method is used to avoid losing references to abilities and effects. If we were to allow abilities and
 * effects to be granted/applied independently, we may not have handles to safely remove them later. This also forces
 * the ability system to be coupled with their source. E.g. each challenger has an ability set, each weapon has an
 * ability set, each power-up has an ability set, etc. This method also prevents any "floating" abilities that are
 * difficult to track. E.g. abilities can't be granted via hard-coding, making them impossible to find when debugging.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UCrashAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Utilities.

public:

	/**
	 * Grants the ability set to the specified ability system component. This grants each ability, applies each effect,
	 * and adds each attribute set in this ability set to the given ASC.
	 *
	 * @param AbilitySystemToGiveTo		The ASC to grant this ability set to.
	 * @param OutGrantedHandles			A collection of handles to the granted abilities, effects, and attribute sets. 
										This can be used to remove these later.
	 * @param SourceObject				(Optional) The object responsible for granting this ability set.
	 */
	void GiveToAbilitySystem(UCrashAbilitySystemComponent* AbilitySystemToGiveTo, FCrashAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;



	// Granted collections.

public:

	/** The gameplay abilities to grant when this ability set is given to an ASC. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<FCrashAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	/** The gameplay effects to apply when this ability set is given to an ASC. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<FCrashAbilitySet_GameplayEffect> GrantedGameplayEffects;

	/** The attribute sets to add when this ability set is given to an ASC. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets")
	TArray<FCrashAbilitySet_AttributeSet> GrantedAttributeSets;
};
