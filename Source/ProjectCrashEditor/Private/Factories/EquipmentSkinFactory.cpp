// Copyright Samuel Reitich. All rights reserved.


#include "Factories/EquipmentSkinFactory.h"

#include "Equipment/EquipmentSkin.h"


UEquipmentSkinFactory::UEquipmentSkinFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UEquipmentSkin::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UEquipmentSkinFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UEquipmentSkin>(InParent, Class, Name, Flags);
}

bool UEquipmentSkinFactory::CanCreateNew() const
{
	return true;
}
