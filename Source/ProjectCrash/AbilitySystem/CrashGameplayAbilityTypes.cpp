// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGameplayAbilityTypes.h"

#include "Characters/CrashCharacter.h"

void FCrashGameplayAbilityActorInfo::InitFromActor(AActor *InOwnerActor, AActor *InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	FGameplayAbilityActorInfo::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

	// If the avatar is a CrashCharacter, cache both its first-person and third-person mesh components.
	if (const ACrashCharacter* AvatarAsCrashChar = InAvatarActor ? Cast<ACrashCharacter>(InAvatarActor) : nullptr)
	{
		SkeletalMeshComponent = AvatarAsCrashChar->GetFirstPersonMesh();
		FirstPersonSkeletalMeshComponent = AvatarAsCrashChar->GetThirdPersonMesh();
	}
}
