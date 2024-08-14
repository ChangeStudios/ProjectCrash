// Copyright Samuel Reitich. All rights reserved.


#include "CrashStatics.h"

#include "GameFramework/PlayerState.h"

APlayerState* UCrashStatics::GetPlayerStateFromObject(UObject* Object)
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

APlayerController* UCrashStatics::GetPlayerControllerFromObject(UObject* Object)
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

AController* UCrashStatics::GetControllerFromObject(UObject* Object)
{
	// Object is a controller.
	if (AController* C = Cast<AController>(Object))
	{
		return C;
	}

	// Object is a player state.
	if (const APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetOwningController();
	}

	// Object is a pawn.
	if (const APawn* TargetPawn = Cast<APawn>(Object))
	{
		return TargetPawn->GetController();
	}

	return nullptr;
}
