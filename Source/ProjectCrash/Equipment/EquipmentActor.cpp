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
	// Hide this actor. It will be revealed once it's fully equipped.
	if (RootComponent)
	{
		RootComponent->SetHiddenInGame(true);
	}

	Super::BeginPlay();
}

void AEquipmentActor::SetEquipmentPerspective(EEquipmentPerspective InEquipmentPerspective)
{
	EquipmentPerspective = InEquipmentPerspective;
}

void AEquipmentActor::OnRep_EquipmentPerspective()
{
	
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
