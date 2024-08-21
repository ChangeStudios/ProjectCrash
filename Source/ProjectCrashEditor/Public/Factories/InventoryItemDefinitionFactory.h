// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Factories/Factory.h"
#include "InventoryItemDefinitionFactory.generated.h"

class UInventoryItemDefinition;

/**
 * UInventoryItemDefinition factory.
 */
UCLASS(HideCategories = Object, MinimalAPI)
class UInventoryItemDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:

	UInventoryItemDefinitionFactory(const FObjectInitializer& ObjectInitializer);

	/** The type of blueprint that will be created. */
	UPROPERTY(EditAnywhere, Category = "InventoryItemDefinitionFactory")
	TSubclassOf<UInventoryItemDefinition> ItemDefinitionClass;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	//~ Begin UFactory Interface
};
