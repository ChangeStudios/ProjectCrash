// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentActor.h"

#include "Net/UnrealNetwork.h"

AEquipmentActor::AEquipmentActor()
{
	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void AEquipmentActor::BeginPlay()
{
	// Re-initialize this actor's visibility if its perspective was initialized before it was attached to a mesh.
	OnRep_EquipmentPerspective();

	Super::BeginPlay();
}

void AEquipmentActor::SetEquipmentPerspective(EEquipmentPerspective InEquipmentPerspective)
{
	EquipmentPerspective = InEquipmentPerspective;

	// Server-side OnRep.
	OnRep_EquipmentPerspective();
}

void AEquipmentActor::OnRep_EquipmentPerspective()
{
	/* Initialize this actor's visibility, which is toggled by perspective changes, with the visibility of the actor to
	 * which it's attached. E.g. if this actor is attached to a first-person mesh, which is hidden because the local
	 * player is in a third-person perspective, then this actor should also be hidden. */
	if (RootComponent && RootComponent->GetAttachParent())
	{
		RootComponent->SetVisibility(RootComponent->GetAttachParent()->IsVisible());

		float FirstPersonDepthScale = (EquipmentPerspective == EEquipmentPerspective::FirstPerson ? 1.0f : 0.0f);
		TArray<UMeshComponent*> MeshComponents;
		GetComponents(UMeshComponent::StaticClass(), MeshComponents);
		for (UMeshComponent* MeshComp : MeshComponents)
		{
			// Enable first-person depth scale for this actor. This prevents clipping on first-person actors.
			MeshComp->SetScalarParameterValueOnMaterials(FName("FirstPerson"), FirstPersonDepthScale);

			// Disable shadows on first-person actors.
			MeshComp->SetCastShadow(!FirstPersonDepthScale);

			/* Force mesh components to use their parent's bounds in first-person. This prevents them from being culled
			 * when they shouldn't be visible (e.g. when completely behind a wall), but still are visible because of our
			 * first-person depth scale. */
			MeshComp->bUseAttachParentBound = FirstPersonDepthScale;
		}
	}
}

void AEquipmentActor::ProcessEquipmentAnimation(FGameplayTag AnimationTag)
{
	/* If the animation tag matches a tag in EquipmentAnimations, play that equipment animation on any skeletal mesh
	 * component on this actor. */
	if (UAnimMontage* EquipmentAnimation = *EquipmentAnimations.Find(AnimationTag))
	{
		TArray<UActorComponent*> Components;
		GetComponents(USkeletalMeshComponent::StaticClass(), Components);

		for (UActorComponent* MeshComp : Components)
		{
			Cast<USkeletalMeshComponent>(MeshComp)->PlayAnimation(EquipmentAnimation, EquipmentAnimation->bLoop);
		}
	}

	// Forward the animation to an optional blueprint implementation.
	K2_ProcessEquipmentAnimation(AnimationTag);
}

void AEquipmentActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, EquipmentPerspective);
}
