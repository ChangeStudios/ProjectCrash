// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CrashAbilityMessage.generated.h"

/**
 * A message involving the behavior of a gameplay ability. Provides information about the ability, along with a
 * tag defining the type of message itself (e.g. "Activated," "Ended," "Cancelled," etc.).
 */
USTRUCT(BlueprintType)
struct FCrashAbilityMessage
{
	GENERATED_BODY()

	/** The type of message: "ability activated," "ability ended," etc. */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	FGameplayTag MessageType;

	/** The ability referred to by the message. */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	FGameplayAbilitySpecHandle AbilitySpecHandle;

	/** The actor info of this message's ability, with which this message was broadcast. */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	FGameplayAbilityActorInfo ActorInfo;

	/** An optional numeric value to pass as a payload, such as the duration of a cooldown applied. */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	float OptionalMagnitude = 0.0f;

	/** Returns a debug string representation of this message. */
	PROJECTCRASH_API FString ToString() const;
};
