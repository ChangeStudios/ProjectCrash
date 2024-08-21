// Copyright Samuel Reitich. All rights reserved.


#include "Factories/EquipmentDefinitionFactory.h"

#include "Equipment/EquipmentDefinition.h"

UEquipmentDefinitionFactory::UEquipmentDefinitionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UEquipmentDefinition::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UEquipmentDefinitionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UEquipmentDefinitionFactory>(InParent, Class, Name, Flags);
}

bool UEquipmentDefinitionFactory::CanCreateNew() const
{
	return true;
}
