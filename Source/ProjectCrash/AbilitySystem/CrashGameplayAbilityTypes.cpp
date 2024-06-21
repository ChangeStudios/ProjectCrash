// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/CrashGameplayAbilityTypes.h"

#include "Characters/CrashCharacterBase_DEP.h"

void FCrashGameplayAbilityActorInfo::InitFromActor(AActor *InOwnerActor, AActor *InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
	FGameplayAbilityActorInfo::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

	/* If the avatar is an ACrashCharacterBase, use the third-person mesh as the skeletal mesh instead of the one found
	 * with the component search in the Super function. */
	AActor* AvatarActorPtr = AvatarActor.Get();
	if (const ACrashCharacterBase_DEP* CrashAvatarPtr = AvatarActorPtr ? Cast<ACrashCharacterBase_DEP>(AvatarActorPtr) : nullptr)
	{
		SkeletalMeshComponent = CrashAvatarPtr->GetThirdPersonMesh();
	}
}
