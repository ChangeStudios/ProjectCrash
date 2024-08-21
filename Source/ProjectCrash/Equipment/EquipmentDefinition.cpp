// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentDefinition.h"

#include "EquipmentInstance.h"

UEquipmentDefinition::UEquipmentDefinition()
{
	EquipmentInstanceClass = UEquipmentInstance::StaticClass();
}
