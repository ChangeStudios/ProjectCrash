// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ActionSetFactory.generated.h"

/**
 * UGameFeatureActionSet factory.
 */
UCLASS()
class PROJECTCRASHEDITOR_API UActionSetFactory : public UFactory
{
	GENERATED_BODY()

public:

	UActionSetFactory(const FObjectInitializer& ObjectInitializer);

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};
