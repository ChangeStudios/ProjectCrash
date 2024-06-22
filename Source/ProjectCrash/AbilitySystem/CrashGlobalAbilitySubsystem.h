// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrashGlobalAbilitySubsystem.generated.h"

class UCrashAbilitySystemComponent;
class UGameplayAbility;

/**
 * A structure containing a globally granted gameplay ability and handles it for each ASC to which it's currently
 * granted.
 */
USTRUCT()
struct FGloballyGrantedAbility
{
	GENERATED_BODY()

public:

	/** The globally granted gameplay ability. */
	UPROPERTY()
	TSubclassOf<UGameplayAbility> GrantedAbility;

	/** Each ASC the ability is granted to, mapped to a handle to the ability in that ASC. */
	UPROPERTY()
	TMap<TObjectPtr<UCrashAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	/** Grants this ability to the given ASC. */
	void GrantToASC(UCrashAbilitySystemComponent* ASC);

	/** Removes this ability from the given ASC. */
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
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem")
	void GrantGlobalAbility(TSubclassOf<UGameplayAbility> Ability);

	/** Removes a granted gameplay ability from all registered ASCs. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GlobalAbilitySubsystem")
	void RemoveGlobalAbility(TSubclassOf<UGameplayAbility> Ability);



	// ASC management.

public:

	/** Registers the given ASC with the global ability subsystem, granting any current global abilities. */
	void RegisterASC(UCrashAbilitySystemComponent* ASC);

	/** Unregisters the given ASC from the global ability subsystem, removing any current global abilities. */
	void UnregisterASC(UCrashAbilitySystemComponent* ASC);

private:

	/** A map containing every current global ability, and handles to the ability in each ASC to which it's granted. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGloballyGrantedAbility> GrantedAbilities;

	/** All ability system components currently registered with the global ability subsystem. */
	UPROPERTY()
	TArray<TObjectPtr<UCrashAbilitySystemComponent>> RegisteredASCs;
};
