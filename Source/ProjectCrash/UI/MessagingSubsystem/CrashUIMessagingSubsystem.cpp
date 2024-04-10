// Copyright Samuel Reitich 2024.


#include "UI/MessagingSubsystem/CrashUIMessagingSubsystem.h"

bool UCrashUIMessagingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<ULocalPlayer>(Outer)->GetGameInstance()->IsDedicatedServerInstance())
	{
		TArray<UClass*> OutChildClasses;
		GetDerivedClasses(GetClass(), OutChildClasses, false);

		// Don't create an instance if there is an overriding implementation somewhere else.
		return OutChildClasses.Num() == 0;
	}

	return false;
}

void UCrashUIMessagingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}
