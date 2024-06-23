// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "CrashGameplayAbilityTypes.generated.h"

class ACrashPlayerState;
class ACrashPlayerController;
class ACrashCharacter;
class UCrashAbilitySystemComponent;

/**
 * Actor info structure for this project. Adds properites and accessors for project-specific actors, and adds separate
 * first-person and third-person skeletal mesh component and animation instance properties for avatars of the
 * CrashCharacter type.
 *
 * Note that this struct's default SkeletalMeshComponent and AnimInstance are the third-person mesh and animation
 * instance. This struct only adds first-person properties, but also ensures the correct data is cached for the
 * third-person properties.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FCrashGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	FCrashGameplayAbilityActorInfo() {}



	// Initialization.

public:

	/** Caches as much actor information as possible. It's rare that we fill out this entire struct. */
	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;

	// Clears cached actor info.
	virtual void ClearActorInfo() override;



	// Custom properties.

// Project-specific actors.
public:

	/** The ability's ASC, cached as a CrashAbilitySystemComponent. Should never be null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<UCrashAbilitySystemComponent> CrashAbilitySystemComponent;

	/** The player controller associated with the owning actor, cached as a CrashPlayerController. Often null. Always
	 * null on simulated proxies. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashPlayerController> CrashPlayerController;

	/** The player state for the controller associated with the owning actor, if it has one. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashPlayerState> CrashPlayerState;

	/** The avatar as a CrashCharacter. Often null. Always null if the avatar is not of type CrashCharacter. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashCharacter> CrashCharacter;

	/** First-person mesh of the avatar actor, if the avatar is of type CrashCharacter. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<USkeletalMeshComponent> FirstPersonSkeletalMeshComponent;

	/** Animation instance of the avatar actor's first-person mesh, if the avatar is of type CrashCharacter. Often
	 * null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<UAnimInstance> FirstPersonAnimInstance;

// Accessors.
public:

	/** Returns the actor info's ability system component as a CrashAbilitySystemComponent. Should never be null. */
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

	/** Returns the actor info's current player controller, as a CrashPlayerController. Often null. Always null on
	 * simulated proxies. */
	ACrashPlayerController* GetCrashPlayerController() const;

	/** Returns the actor info's current player state, as a CrashPlayerState. Often null. */
	ACrashPlayerState* GetCrashPlayerState() const;

	/** Returns the affected animation instance from the FirstPersonSkeletalMeshComponent. Null for avatars that are not
	 * of type CrashCharacter. */
	UAnimInstance* GetFirstPersonAnimInstance() const;
};