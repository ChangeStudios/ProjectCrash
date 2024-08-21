// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Factories/Factory.h"
#include "EquipmentDefinitionFactory.generated.h"

/**
 * UEquipmentDefinition factory.
 */
UCLASS(MinimalAPI)
class UEquipmentDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:

	UEquipmentDefinitionFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};
