// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/Messages/CrashVerbMessageHelpers.h"

#include "CrashAbilityMessage.h"
#include "CrashVerbMessage.h"
#include "GameFramework/PlayerState.h"

/**
 * FCrashAbilityMessage
 */
FString FCrashAbilityMessage::ToString() const
{
	FString HumanReadableMessage;
	FCrashAbilityMessage::StaticStruct()->ExportText(/* Out */ HumanReadableMessage, this, /* Defaults= */ nullptr, /** OwnerObject= */ nullptr, PPF_None, /* ExportRootScope= */ nullptr);
	return HumanReadableMessage;
}



/**
 * FCrashVerbMessage
 */
FString FCrashVerbMessage::ToString() const
{
	FString HumanReadableMessage;
	FCrashVerbMessage::StaticStruct()->ExportText(/* Out */ HumanReadableMessage, this, /* Defaults= */ nullptr, /* OwnerObject= */ nullptr, PPF_None, /* ExportRootScope= */ nullptr);
	return HumanReadableMessage;
}



/**
 * UCrashVerbMessageHelpers
 */
APlayerState* UCrashVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	// Object is a controller.
	if (const AController* C = Cast<AController>(Object))
	{
		return C->PlayerState;
	}

	// Object is a player state.
	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}

	// Object is a pawn.
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
	// Object is a player controller.
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	// Object is a player state.
	if (const APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetPlayerController();
	}

	// Object is a pawn.
	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}
