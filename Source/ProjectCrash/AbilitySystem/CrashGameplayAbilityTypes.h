// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CrashGameplayAbilityTypes.generated.h"

/**
 * Actor info structure for this project. Adds separate first-person and third-person skeletal mesh component properties
 * for avatars of the CrashCharacter type.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FCrashGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	FCrashGameplayAbilityActorInfo() {};

	/** First-person mesh of the avatar actor. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<USkeletalMeshComponent> FirstPersonSkeletalMeshComponent;

	/** Caches the first- and third-person meshes if the avatar is a CrashCharacter. Otherwise, tries to cache any
	 * skeletal mesh component as SkeletalMeshComponent, and leaves the first-person component null. */
	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
};