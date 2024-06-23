// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CrashGameplayAbilityTypes.generated.h"

/**
 * Actor info structure for this project. Adds separate first-person and third-person skeletal mesh component and
 * animation instance properties for avatars of the CrashCharacter type.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FCrashGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	FCrashGameplayAbilityActorInfo() {};

	/** First-person mesh of the avatar actor. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<USkeletalMeshComponent> FirstPersonSkeletalMeshComponent;

	/** Animation instance of the avatar actor's first-person mesh. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<UAnimInstance> FirstPersonAnimInstance;

	/** Accessor to get the affected animation instance from the FirstPersonSkeletalMeshComponent. */
	UAnimInstance* GetFirstPersonAnimInstance() const;

	/** Caches the first-person and third-person meshes and animation instances if the avatar is a CrashCharacter.
	 * Otherwise, leaves any mesh components and animation instances that could be found as the SkeletalMeshComponent
	 * and AnimInstance properties, and leaves the first-person properties null. */
	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
};