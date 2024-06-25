// Copyright Samuel Reitich. All rights reserved.


#include "Public/Factories/GameModeDataFactory.h"

#include "GameFramework/GameModes/CrashGameModeData.h"

UGameModeDataFactory::UGameModeDataFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UCrashGameModeData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGameModeDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCrashGameModeData>(InParent, Class, Name, Flags);
}

bool UGameModeDataFactory::CanCreateNew() const
{
	return true;
}
