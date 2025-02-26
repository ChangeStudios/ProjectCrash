// Copyright Samuel Reitich. All rights reserved.


#include "Animation/AnimNotifies/AnimNotifyState_SpawnAnimActor.h"


UAnimNotifyState_SpawnAnimActor::UAnimNotifyState_SpawnAnimActor()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 25, 150);
#endif
}

void UAnimNotifyState_SpawnAnimActor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Retrieve the world to spawn our actor.
	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	// Make sure we have a mesh to spawn and a mesh comp to attach it to.
	if (!MeshToSpawn || !MeshComp)
	{
		if (!World->IsPreviewWorld())
		{
			UE_LOG(LogAnimation, Error, TEXT("Attempted to spawn animation actor in animation (%s), but no mesh to spawn was specified."), *GetNameSafe(Animation));
		}

		return;
	}
}
