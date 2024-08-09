// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Factories/Factory.h"
#include "InputActionMappingFactory.generated.h"

/**
 * UCrashInputActionMapping factory.
 */
UCLASS(MinimalAPI)
class UInputActionMappingFactory : public UFactory
{
	GENERATED_BODY()

public:

	UInputActionMappingFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};
