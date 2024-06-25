// Copyright Samuel Reitich. All rights reserved.


#include "Factories/ActionSetFactory.h"

#include "GameFramework/GameFeatures/GameFeatureActionSet.h"

UActionSetFactory::UActionSetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UGameFeatureActionSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UActionSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UGameFeatureActionSet>(InParent, Class, Name, Flags);
}

bool UActionSetFactory::CanCreateNew() const
{
	return true;
}
