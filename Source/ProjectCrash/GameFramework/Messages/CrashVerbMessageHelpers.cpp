// Copyright Samuel Reitich 2024.


#include "GameFramework/Messages/CrashVerbMessageHelpers.h"

#include "CrashVerbMessage.h"
#include "GameFramework/PlayerState.h"

FString FCrashVerbMessage::ToString() const
{
	FString HumanReadableMessage;
	FCrashVerbMessage::StaticStruct()->ExportText(/* Out */ HumanReadableMessage, this, /* Defaults= */ nullptr, /* OwnerObject= */ nullptr, PPF_None, /* ExportRootScope= */ nullptr);
	return HumanReadableMessage;
}

APlayerState* UCrashVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	if (const APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC->PlayerState;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}
	
	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		if (APlayerState* TargetPS = TargetPawn->GetPlayerState())
		{
			return TargetPS;
		}
	}

	return nullptr;
}

APlayerController* UCrashVerbMessageHelpers::GetPlayerControllerFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	if (const APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetPlayerController();
	}

	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}
