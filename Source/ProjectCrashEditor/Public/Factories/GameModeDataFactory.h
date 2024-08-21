// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "Factories/Factory.h"
#include "GameModeDataFactory.generated.h"

/**
 * UGameModeData factory.
 */
UCLASS(MinimalAPI)
class UGameModeDataFactory : public UFactory
{
	GENERATED_BODY()

public:

	UGameModeDataFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};
