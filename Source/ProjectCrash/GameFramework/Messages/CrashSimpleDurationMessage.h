// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CrashSimpleDurationMessage.generated.h"

/**
 * A simple message containing a target actor and a magnitude, intended for communicating simple duration-based
 * messages, such as a respawn timer.
 */
USTRUCT(BlueprintType)
struct FCrashSimpleDurationMessage
{
	GENERATED_BODY()

	/** The actor that cares about this message. Doesn't have to be the instigator. Could be a target, source, etc. */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	/** Optional duration of this message. */
	UPROPERTY(BlueprintReadWrite)
	float Duration = 0.0f;

	/** Optional result of this message. */
	UPROPERTY(BlueprintReadWrite)
	bool bResult = false;
};
