// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentActor.h"

AEquipmentActor::AEquipmentActor()
{
	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void AEquipmentActor::BeginPlay()
{
	// Hide this actor It will be revealed once it's fully equipped.
	if (RootComponent)
	{
		RootComponent->SetHiddenInGame(true);
	}

	Super::BeginPlay();
}

void AEquipmentActor::InitEquipmentPerspective(EEquipmentPerspective InEquipmentPerspective)
{
}

void AEquipmentActor::ProcessEquipmentEvent(FGameplayTag Event)
{
	// TODO
}
