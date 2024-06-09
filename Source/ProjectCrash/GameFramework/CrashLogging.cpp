// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashLogging.h"

DEFINE_LOG_CATEGORY(LogCrash);
DEFINE_LOG_CATEGORY(LogCrashGameMode);
DEFINE_LOG_CATEGORY(LogEquipment);
DEFINE_LOG_CATEGORY(LogUserInterface);

FString GetClientServerContextString(UObject* ContextObject)
{
	ENetRole Role = ROLE_None;

	// Context object is an actor.
	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		Role = Actor->GetLocalRole();
	}
	// Context actor is an actor component.
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return (Role == ROLE_Authority) ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
