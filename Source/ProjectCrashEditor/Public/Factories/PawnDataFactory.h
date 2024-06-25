// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PawnDataFactory.generated.h"

/**
 * UPawnData factory.
 */
UCLASS()
class PROJECTCRASHEDITOR_API UPawnDataFactory : public UFactory
{
	GENERATED_BODY()

public:

	UPawnDataFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};
