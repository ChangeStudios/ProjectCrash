// Copyright Samuel Reitich. All rights reserved.


#include "Factories/CrashAbilitySetFactory.h"

#include "AbilitySystem/Abilities/CrashAbilitySet.h"

UCrashAbilitySetFactory::UCrashAbilitySetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UCrashAbilitySet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UCrashAbilitySetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCrashAbilitySet>(InParent, Class, Name, Flags);
}

bool UCrashAbilitySetFactory::CanCreateNew() const
{
	return true;
}
