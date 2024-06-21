// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"

struct PROJECTCRASH_API FCrashGameplayAbilityActorInfo : FGameplayAbilityActorInfo
{
	/** Attempts to find the third-person mesh from the avatar actor if it's an ACrashCharacterBase. Otherwise, falls
	 * back to any skeletal mesh component in the avatar actor. */
	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
};