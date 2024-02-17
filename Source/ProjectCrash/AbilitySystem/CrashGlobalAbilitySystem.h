// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrashGlobalAbilitySystem.generated.h"

class UCrashAbilitySystemComponent;
class UGameplayAbility;

/**
 * A structure containing a globally granted gameplay ability and the handles of the ability in each ASC to which it's
 * currently granted.
 */
USTRUCT()
struct FGlobalGrantedAbilityList
{
	GENERATED_BODY()

public:

	/** The globally granted gameplay ability. */
	UPROPERTY()
	TSubclassOf<UGameplayAbility> GrantedAbility;

	/** Each ASC the ability is granted to, mapped to a handle to that ability in that ASC. */
	UPROPERTY()
	TMap<TObjectPtr<UCrashAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	/** Grants this ability to the given ASC. */
	void GrantToASC(UCrashAbilitySystemComponent* ASC);

	/** Removes this ability from the given ASC. */
	void RemoveFromASC(UCrashAbilitySystemComponent* ASC);
};


/**
 * A global object that can grant gameplay abilities globally.
 */
UCLASS()
class PROJECTCRASH_API UCrashGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	// Initialization.

public:

	/** Default constructor. */
	UCrashGlobalAbilitySystem();



	// Ability system.

// Abilities.
public:

	/** Grants a gameplay ability to all registers ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability System")
	void GrantGlobalAbility(TSubclassOf<UGameplayAbility> Ability);

	/** Removes a granted gameplay ability from all registers ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability System")
	void RemoveGlobalAbility(TSubclassOf<UGameplayAbility> Ability);

// ASC management.
public:

	/** Registers the given ASC with the global ability system, granting any ongoing global abilities. */
	void RegisterASC(UCrashAbilitySystemComponent* ASC);

	/** Unregisters the given ASC from the global ability system, removing any granted global abilities. */
	void UnregisterASC(UCrashAbilitySystemComponent* ASC);

private:

	/** A map containing every currently granted global ability and handles to the ability in each ASC to which it's
	 * granted. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalGrantedAbilityList> GrantedAbilities;

	/** All ability system components currently registered with the global ability system. */
	UPROPERTY()
	TArray<TObjectPtr<UCrashAbilitySystemComponent>> RegisteredASCs;
};
