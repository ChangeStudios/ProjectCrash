// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGameplayAbilityTypes.h"

#include "Characters/CrashCharacter.h"

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

void FCrashGameplayAbilityActorInfo::InitFromActor(AActor *InOwnerActor, AActor *InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	FGameplayAbilityActorInfo::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

	/* If the avatar is a CrashCharacter, cache both its first-person and third-person mesh components and animation
	 * instances. Otherwise, it will use anything it can find for its third-person properties, and leave the
	 * first-person properties null. */
	if (const ACrashCharacter* AvatarAsCrashChar = InAvatarActor ? Cast<ACrashCharacter>(InAvatarActor) : nullptr)
	{
		SkeletalMeshComponent = AvatarAsCrashChar->GetThirdPersonMesh();
		FirstPersonSkeletalMeshComponent = AvatarAsCrashChar->GetFirstPersonMesh();

		FirstPersonAnimInstance = FirstPersonSkeletalMeshComponent->GetAnimInstance();
	}

	// Always initialize AnimInstance, since the Super doesn't.
	AnimInstance = SkeletalMeshComponent->GetAnimInstance();
}
