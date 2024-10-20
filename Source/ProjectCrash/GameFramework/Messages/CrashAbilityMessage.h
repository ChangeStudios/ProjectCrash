// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "CrashAbilityMessage.generated.h"

/**
 * A message involving the behavior of a gameplay ability. Provides standardized information about the ability and an
 * identifying message type.
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

	/** The actor info of the ability's owning ASC at the time of the broadcast. */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	FCrashGameplayAbilityActorInfo ActorInfo;

	/** An optional numeric value to pass as a payload (duration of a cooldown, ability's level, etc.). */
	UPROPERTY(BlueprintReadWrite, Category = Gameplay)
	float Magnitude = 0.0f;

	/** Returns a debug string representation of this message. */
	PROJECTCRASH_API FString ToString() const
	{
		UAbilitySystemComponent* ASC = ActorInfo.AbilitySystemComponent.Get();
		FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromHandle(AbilitySpecHandle) : nullptr;

		return FString::Printf(TEXT("Ability [%s] triggered [%s] with a magnitude of %f (Owner: [%s], Avatar: [%s])"),
			Spec ? *GetNameSafe(Spec->Ability) : *FString("Spec Not Found"),
			*MessageType.ToString(),
			Magnitude,
			*GetNameSafe(ActorInfo.OwnerActor.Get()),
			*GetNameSafe(ActorInfo.AvatarActor.Get()));
	}
};
