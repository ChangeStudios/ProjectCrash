// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "CrashAbilitySystemGlobals.generated.h"

class UCrashAbilitySystemComponent;

/**
 * Defines global properties of the ability system and provides various utilities.
 */
UCLASS(Config = Game)
class PROJECTCRASH_API UCrashAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

private:

	/** Tells the system to create effect contexts of our custom type: FCrashGameplayEffectContext. */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;

	/** Tells the system to create ability actor info structs of our custom type: FCrashGameplayAbilityActorInfo. */
	virtual FGameplayAbilityActorInfo* AllocAbilityActorInfo() const override;

public:

	/**
	 * Searches the given actor for a Crash ASC using the IAbilitySystemInterface. Will revert to a standard
	 * component search if the interface fails or is not implemented.
	 *
	 * @param Actor				The actor to search.
	 * @param LookForComponent	Whether to perform a standard component search if the ability interface fails.
	 * @return					A CrashAbilitySystemComponent if one was found. Returns nullptr if no ASC was found or
	 *							if the ASC was not a CrashAbilitySystemComponent.
	 */
	static UCrashAbilitySystemComponent* GetCrashAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = true);
	
};
