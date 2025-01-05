// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGameplayAbilityTypes.h"

#include "Characters/CrashCharacter.h"
#include "Components/CrashAbilitySystemComponent.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"

void FCrashGameplayAbilityActorInfo::InitFromActor(AActor *InOwnerActor, AActor *InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	FGameplayAbilityActorInfo::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

	CrashAbilitySystemComponent = Cast<UCrashAbilitySystemComponent>(InAbilitySystemComponent);
	checkf(CrashAbilitySystemComponent.IsValid(), TEXT("Actor info initialized without an ASC of type CrashAbilitySystemComponent. Please use CrashAbilitySystemComponent for all ASCs in this project."));

	/* If the avatar is a CrashCharacter, cache both its first-person and third-person mesh components and animation
	 * instances. Otherwise, it will use anything it can find for its third-person properties, and leave the
	 * first-person properties null. */
	if (ACrashCharacter* AvatarAsCrashChar = InAvatarActor ? Cast<ACrashCharacter>(InAvatarActor) : nullptr)
	{
		// Cache the avatar as a CrashCharacter.
		CrashCharacter = AvatarAsCrashChar;

		// Cache the first-person mesh.
		SkeletalMeshComponent = AvatarAsCrashChar->GetThirdPersonMesh();
		FirstPersonSkeletalMeshComponent = AvatarAsCrashChar->GetFirstPersonMesh();
	}

	// Cache the typed player controller and player state.
	if (PlayerController.IsValid())
	{
		CrashPlayerController = Cast<ACrashPlayerController>(PlayerController);
		CrashPlayerState = PlayerController.Get()->GetPlayerState<ACrashPlayerState>();
	}
	// If the player controller isn't valid, try to cache the player state directly from the avatar.
	else
	{
		if (APawn* AvatarAsPawn = Cast<APawn>(AvatarActor))
		{
			CrashPlayerState = AvatarAsPawn->GetPlayerState<ACrashPlayerState>();
		}
	}
}

void FCrashGameplayAbilityActorInfo::ClearActorInfo()
{
	FGameplayAbilityActorInfo::ClearActorInfo();

	// Clear all cached data.
	AnimInstance = nullptr;
	FirstPersonSkeletalMeshComponent = nullptr;
	CrashCharacter = nullptr;
	CrashPlayerState = nullptr;
	CrashPlayerController = nullptr;
	CrashAbilitySystemComponent = nullptr;
}

UCrashAbilitySystemComponent* FCrashGameplayAbilityActorInfo::GetCrashAbilitySystemComponent() const
{
	return CrashAbilitySystemComponent.IsValid() ? CrashAbilitySystemComponent.Get() : nullptr;
}

ACrashPlayerController* FCrashGameplayAbilityActorInfo::GetCrashPlayerController() const
{
	return CrashPlayerController.IsValid() ? CrashPlayerController.Get() : nullptr;
}

ACrashPlayerState* FCrashGameplayAbilityActorInfo::GetCrashPlayerState() const
{
	return CrashPlayerState.IsValid() ? CrashPlayerState.Get() : nullptr;
}

UAnimInstance* FCrashGameplayAbilityActorInfo::GetFirstPersonAnimInstance() const
{
	const USkeletalMeshComponent* SKMC = FirstPersonSkeletalMeshComponent.Get();

	if (SKMC)
	{
		if (AffectedAnimInstanceTag != NAME_None)
		{
			if(UAnimInstance* Instance = SKMC->GetAnimInstance())
			{
				return Instance->GetLinkedAnimGraphInstanceByTag(AffectedAnimInstanceTag);
			}
		}

		return SKMC->GetAnimInstance();
	}

	return nullptr;
}