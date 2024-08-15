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
	/** Hide this actor; it will be revealed once it's been fully replicated. We don't have to worry about this on
	 * authority. */
	if (!HasAuthority() && RootComponent)
	{
		RootComponent->SetHiddenInGame(true);
	}

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
	RootComponent->SetVisibility(RootComponent->GetAttachParent()->IsVisible());
}

void AEquipmentActor::ProcessEquipmentEvent(FGameplayTag Event)
{
	// TODO: implement
}

void AEquipmentActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, EquipmentPerspective);
}
