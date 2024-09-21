// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameModes/GameModePropertySubsystem.h"

#include "GameFramework/CrashLogging.h"

bool UGameModePropertySubsystem::AddGameModeProperty(FGameplayTag PropertyTag, float Value)
{
	// Don't add properties multiple times.
	if (DoesGameModePropertyExist(PropertyTag))
	{
		UE_LOG(LogCrashGameMode, Error, TEXT("Attempted to register game mode property [%s] with value [%f], but property is already defined. Existing value: [%f]."), *PropertyTag.ToString(), Value, GetGameModeProperty(PropertyTag));
		return false;
	}

	// Register the tag.
	if (ensure(PropertyTag.IsValid()))
	{
		GameModeProperties.Add(PropertyTag, Value);
		return true;
	}

	return false;
}

bool UGameModePropertySubsystem::RemoveGameModeProperty(FGameplayTag Property)
{
	if (DoesGameModePropertyExist(Property))
	{
		GameModeProperties.FindAndRemoveChecked(Property);
		return true;
	}

	return false;
}

float UGameModePropertySubsystem::GetGameModeProperty(FGameplayTag Property) const
{
	if (GameModeProperties.Contains(Property))
	{
		return GameModeProperties[Property];
	}

	return 1.0f;
}

bool UGameModePropertySubsystem::DoesGameModePropertyExist(FGameplayTag Property) const
{
	if (ensure(Property.IsValid()))
	{
		return GameModeProperties.Contains(Property);
	}

	return false;
}

float UGameModePropertySubsystem::GetGameModeProperty(const UObject* WorldContextObject, FGameplayTag Property)
{
	// Static wrapper for GetGameModeProperty.
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameModePropertySubsystem* GameModePropertySubsystem = World->GetSubsystem<UGameModePropertySubsystem>())
		{
			return GameModePropertySubsystem->GetGameModeProperty(Property);
		}
	}

	return 1.0f;
}

bool UGameModePropertySubsystem::DoesGameModePropertyExist(const UObject* WorldContextObject, FGameplayTag Property)
{
	// Static wrapper for DoesGameModePropertyExist.
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameModePropertySubsystem* GameModePropertySubsystem = World->GetSubsystem<UGameModePropertySubsystem>())
		{
			return GameModePropertySubsystem->DoesGameModePropertyExist(Property);
		}
	}

	return false;
}
