// Copyright Samuel Reitich. All rights reserved.


#include "Factories/UserFacingGameModeDataFactory.h"

#include "GameFramework/GameModes/UserFacingGameModeData.h"

UUserFacingGameModeDataFactory::UUserFacingGameModeDataFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUserFacingGameModeData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UUserFacingGameModeDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UUserFacingGameModeData>(InParent, Class, Name, Flags);
}

bool UUserFacingGameModeDataFactory::CanCreateNew() const
{
	return true;
}
