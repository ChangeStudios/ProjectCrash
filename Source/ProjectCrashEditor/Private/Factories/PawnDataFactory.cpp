// Copyright Samuel Reitich. All rights reserved.


#include "Factories/PawnDataFactory.h"

#include "Characters/Data/PawnData.h"

UPawnDataFactory::UPawnDataFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPawnData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPawnDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPawnData>(InParent, Class, Name, Flags);
}

bool UPawnDataFactory::CanCreateNew() const
{
	return true;
}
