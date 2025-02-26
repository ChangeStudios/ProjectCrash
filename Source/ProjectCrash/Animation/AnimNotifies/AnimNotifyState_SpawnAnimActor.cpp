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

	// Spawn a static mesh actor if the mesh to spawn is a static mesh.
	if (MeshToSpawn->IsA(UStaticMesh::StaticClass()))
	{
		if (AStaticMeshActor* SpawnedActorStatic = World->SpawnActorDeferred<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnTransform, MeshComp->GetOwner()))
		{
			SpawnedActorStatic->GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(MeshToSpawn));
			ApplyMaterialOverrides(SpawnedActorStatic->GetStaticMeshComponent());
			SpawnedActor = SpawnedActorStatic;
		}
	}
	// Spawn a skeletal mesh actor if the mesh to spawn is a skeletal mesh.
	else
	{
		if (ASkeletalMeshActor* SpawnedActorSkeletal = World->SpawnActorDeferred<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(), SpawnTransforms, MeshComp->GetOwner()))
		{
			SpawnedActorSkeletal->GetSkeletalMeshComponent()->SetSkeletalMesh(Cast<USkeletalMesh>(MeshToSpawn));
			ApplyMaterialOverrides(SpawnedActorSkeletal->GetSkeletalMeshComponent());
			SpawnedActor = SpawnedActorSkeletal;

			if (SpawnedActorSkeletal)
			{
				SpawnedActorSkeletal->GetSkeletalMeshComponent()->PlayAnimation(ActorAnimation, ActorAnimationLoops);
			}
		}
	}

	// Set up the spawned actor.
	if (ensure(SpawnedActor))
	{
		SpawnedActor->SetActorEnableCollision(false);
		SpawnedActor->AttachToComponent(MeshComp, FAttachmentTransformRule::KeepRelativeTransform, AttachSocket);
		SpawnedActorStatic->FinishSpawning(SpawnTransform);
	}
}

void UAnimNotifyState_SpawnAnimActor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Destroy the animation actor, if one was spawned.
	if (IsValid(SpawnedActor))
	{
		/* We let the actor stick around for a second before destroying it, but we hide its mesh. This is to let any 
		 * other timed events (e.g. a particle effect spawned by the spawned actor's animation) finish before the actor 
		 * is destroyed. */
		SpawnedActor->SetLifeSpan(1.0f);
		SpawnedActor->ForEachComponent(false, [](UActorComponent* InComponent)
		{
			if (UMeshComponent* CompAsMesh = Cast<UMeshComponent>(InComponent))
			{
				CompAsMesh->SetHiddenInGame(true, false);
			}
		});
	}
}

void UAnimNotifyState_SpawnAnimActor::ApplyMaterialOverrides(UMeshComponent* MeshComp)
{
	if (bOverrideMaterials && IsValid(MeshComp))
	{
		const int32 NumOverrideMaterials = FMath::Min(OverrideMaterials.Num(), MeshComp->GetNumMaterials());
		for (int32 OverrideIndex = 0; OverrideIndex < NumOverrideMaterials; ++OverrideIndex)
		{
			if (UMaterialInterface* OverrideMat = OverrideMaterials[OverrideIndex])
			{
				MeshComp->SetMaterial(OverrideIndex, OverrideMat);
			}
		}

		if (OverlayMaterial)
		{
			MeshComp->SetOverlayMaterial(OverlayMaterial);
		}
	}
}