// Copyright Samuel Reitich. All rights reserved.


#include "Factories/InventoryItemDefinitionFactory.h"

#include "Inventory/InventoryItemDefinition.h"
#include "Inventory/InventoryItemDefinitionBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"

UInventoryItemDefinitionFactory::UInventoryItemDefinitionFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UInventoryItemDefinitionBlueprint::StaticClass();
	ItemDefinitionClass = UInventoryItemDefinition::StaticClass();
}

bool UInventoryItemDefinitionFactory::ConfigureProperties()
{
	ItemDefinitionClass = UInventoryItemDefinition::StaticClass();
	return true;
}

UObject* UInventoryItemDefinitionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (ItemDefinitionClass)
	{
		UInventoryItemDefinitionBlueprint* NewBP = CastChecked<UInventoryItemDefinitionBlueprint>(FKismetEditorUtilities::CreateBlueprint(ItemDefinitionClass, InParent, InName, BPTYPE_Normal, UInventoryItemDefinitionBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext));
		return NewBP;
	}

	return NULL;
}

UObject* UInventoryItemDefinitionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}
