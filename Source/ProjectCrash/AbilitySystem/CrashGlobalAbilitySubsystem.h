// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrashGlobalAbilitySubsystem.generated.h"

class UCrashAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;

/**
 * Entry for tracking globally granted gameplay abilities.
 */
USTRUCT()
struct FGloballyGrantedAbility
{
	GENERATED_BODY()

public:

	/** Each ASC to which the ability is granted, mapped to that ASC's handle for the ability. */
	UPROPERTY()
	TMap<TObjectPtr<UCrashAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	/** Grants the ability to the given ASC. */
	void GrantToASC(TSubclassOf<UGameplayAbility> Ability, UCrashAbilitySystemComponent* ASC);

	/** Removes the ability from the given ASC. */
	void RemoveFromASC(UCrashAbilitySystemComponent* ASC);
};



/**
 * Entry for tracking globally applied gameplay effects.
 */
USTRUCT()
struct FGloballyAppliedEffect
{
	GENERATED_BODY()

public:

	/** Each ASC to which the effect is applied, mapped to that ASC's handle for the effect. */
	UPROPERTY()
	TMap<TObjectPtr<UCrashAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	/** Applies the effect to the given ASC. */
	void ApplyToASC(TSubclassOf<UGameplayEffect> Effect, UCrashAbilitySystemComponent* ASC);

	/** Removes the effect from the given ASC. */
	void RemoveFromASC(UCrashAbilitySystemComponent* ASC);
};



/**
 * A subsystem that can grant gameplay abilities globally, to all active ability system components.
 */
UCLASS()
class PROJECTCRASH_API UCrashGlobalAbilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

	// Abilities.

public:

	/** Grants a gameplay ability to all registers ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem", Meta = (Keywords = "add"))
	void GrantGlobalAbility(TSubclassOf<UGameplayAbility> Ability);

	/** Removes a granted gameplay ability from all registered ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem")
	void RemoveGlobalAbility(TSubclassOf<UGameplayAbility> Ability);



	// Effects.

public:

	/** Grants a gameplay effect to all registered ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem", Meta = (Keywords = "add grant"))
	void ApplyGlobalEffect(TSubclassOf<UGameplayEffect> Effect);

	/** Removes an applied gameplay effect from all registered ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem", Meta = (Keywords = "end"))
	void RemoveGlobalEffect(TSubclassOf<UGameplayEffect> Effect);



	// ASC management.

public:

	/** Registers the given ASC with the global ability subsystem, granting any current global abilities. */
	void RegisterASC(UCrashAbilitySystemComponent* ASC);

	/** Unregisters the given ASC from the global ability subsystem, removing any current global abilities. */
	void UnregisterASC(UCrashAbilitySystemComponent* ASC);

private:

	/** A map containing current global abilities, and handles to that ability in each ASC to which it's granted. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGloballyGrantedAbility> GrantedAbilities;

	/** A map containing current global effects, and handles to that effect in each ASC to which it's applied. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGloballyAppliedEffect> AppliedEffects;

	/** All ability system components currently registered with the global ability subsystem. */
	UPROPERTY()
	TArray<TObjectPtr<UCrashAbilitySystemComponent>> RegisteredASCs;
};
