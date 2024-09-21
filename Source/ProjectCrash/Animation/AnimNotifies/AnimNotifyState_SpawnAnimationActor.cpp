// Copyright Samuel Reitich. All rights reserved.


#include "Animation/AnimNotifies/AnimNotifyState_SpawnAnimationActor.h"

#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"

UAnimNotifyState_SpawnAnimationActor::UAnimNotifyState_SpawnAnimationActor() :
	bFirstPerson(false),
	ActorAnimationLoops(false)
{
#if WITH_EDITORONLY_DATA
	// Notify's default color in the editor.
	NotifyColor = FColor(255, 25, 150);
#endif
}

FString UAnimNotifyState_SpawnAnimationActor::GetNotifyName_Implementation() const
{
	if (MeshToSpawn)
	{
		return ("Spawn " + GetNameSafe(MeshToSpawn));
	}
	else
	{
		return "Spawn Animation Actor (Unset)";
	}
}

void UAnimNotifyState_SpawnAnimationActor::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// Make sure we have a mesh to spawn and a mesh comp to attach it to.
	if (!MeshToSpawn || !MeshComp)
	{
		return;
	}

	// Retrieve the world to spawn our actor.
	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	AActor* SpawnedActor = nullptr;
	UMeshComponent* SpawnedMeshComp = nullptr;

	// Spawn a static mesh actor if the mesh to spawn is a static mesh.
	if (MeshToSpawn->IsA(UStaticMesh::StaticClass()))
	{
		if (AStaticMeshActor* SpawnedActorStatic = World->SpawnActorDeferred<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnTransform, MeshComp->GetOwner()))
		{
			SpawnedActorStatic->GetStaticMeshComponent()->SetStaticMesh(Cast<UStaticMesh>(MeshToSpawn));
			SpawnedActor = SpawnedActorStatic;
			SpawnedMeshComp = SpawnedActorStatic->GetStaticMeshComponent();
		}
	}
	// Spawn a skeletal mesh actor if the mesh to spawn is a skeletal mesh.
	else
	{
		if (ASkeletalMeshActor* SpawnedActorSkeletal = World->SpawnActorDeferred<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(), SpawnTransform, MeshComp->GetOwner()))
		{
			SpawnedActorSkeletal->GetSkeletalMeshComponent()->SetSkeletalMesh(Cast<USkeletalMesh>(MeshToSpawn));
			SpawnedActor = SpawnedActorSkeletal;
			SpawnedMeshComp = SpawnedActorSkeletal->GetSkeletalMeshComponent();

			// Play the optional animation for skeletal meshes.
			if (SpawnedActorSkeletal)
			{
				SpawnedActorSkeletal->GetSkeletalMeshComponent()->PlayAnimation(ActorAnimation, ActorAnimationLoops);
			}
		}
	}

	// Set up the spawned actor.
	if (ensure(SpawnedActor))
	{
		SpawnedActor->GetRootComponent()->SetVisibility(MeshComp->IsVisible());
		SpawnedActor->FinishSpawning(SpawnTransform);
		SpawnedActor->SetActorEnableCollision(false);
		SpawnedActor->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
		SpawnedMeshComp->SetScalarParameterValueOnMaterials(FName("FirstPerson"), bFirstPerson ? 1.0f : 0.0f);
		SpawnedMeshComp->bUseAttachParentBound = bFirstPerson;

		SpawnedActors.Add(SpawnedActor);
	}
}

void UAnimNotifyState_SpawnAnimationActor::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// Destroy the animation actor, if one was spawned.
	for (AActor* SpawnedActor : SpawnedActors)
	{
		SpawnedActor->Destroy();
	}

	SpawnedActors.Empty();
}

#if WITH_EDITOR
bool UAnimNotifyState_SpawnAnimationActor::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);

	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		/* Animation properties can't be edited unless this notify spawns a skeletal mesh (static meshes can't play
		 * animations). */
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UAnimNotifyState_SpawnAnimationActor, ActorAnimation) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UAnimNotifyState_SpawnAnimationActor, ActorAnimationLoops))
		{
			bIsEditable = IsValid(MeshToSpawn) && MeshToSpawn->IsA(USkeletalMesh::StaticClass());
		}
	}

	return bIsEditable;
}
#endif // WITH_EDITOR