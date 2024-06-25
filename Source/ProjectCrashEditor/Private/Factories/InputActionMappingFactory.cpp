// Copyright Samuel Reitich. All rights reserved.


#include "Factories/InputActionMappingFactory.h"

#include "Input/CrashInputActionMapping.h"


UInputActionMappingFactory::UInputActionMappingFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UCrashInputActionMapping::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UInputActionMappingFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCrashInputActionMapping>(InParent, Class, Name, Flags);
}

bool UInputActionMappingFactory::CanCreateNew() const
{
	return true;
}
