// Copyright Samuel Reitich. All rights reserved.


#include "Player/CrashPlayerStart.h"

#include "GameFramework/GameModeBase.h"

EPlayerStartLocationOccupancy ACrashPlayerStart::GetLocationOccupancy(AController* const ControllerToFit) const
{
	UWorld* World = GetWorld();

	if (HasAuthority() && World)
	{
		if (AGameModeBase* GM = World->GetAuthGameMode())
		{
			TSubclassOf<APawn> PawnClass = GM->GetDefaultPawnClassForController(ControllerToFit);
			const APawn* const PawnToFit = PawnClass ? GetDefault<APawn>(PawnClass) : nullptr;

			FVector SpawnLocation = GetActorLocation();
			const FRotator SpawnRotation = GetActorRotation();

			// The player's pawn can fit at this player start.
			if (!World->EncroachingBlockingGeometry(PawnToFit, SpawnLocation, SpawnRotation, nullptr))
			{
				return EPlayerStartLocationOccupancy::Empty;
			}
			// The player's pawn can be adjusted to fit at this player start.
			else if (World->FindTeleportSpot(PawnToFit, SpawnLocation, SpawnRotation))
			{
				return EPlayerStartLocationOccupancy::Partial;
			}
		}
	}

	// The player's pawn cannot fit at this player start.
	return EPlayerStartLocationOccupancy::Full;
}

bool ACrashPlayerStart::TryClaim(AController* ClaimingController)
{
	if ((ClaimingController != nullptr) && !IsClaimed())
	{
		// Claim this player start, and start waiting to unclaim it when possible.
		ClaimedController = ClaimingController;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ExpirationTimerHandle, FTimerDelegate::CreateUObject(this, &ACrashPlayerStart::CheckUnclaimed), ExpirationCheckInterval, true, MinExpirationDuration);
		}

		return true;
	}

	return false;
}

void ACrashPlayerStart::CheckUnclaimed()
{
	// Unclaimed when the current claimant has been spawned and is no longer encroaching on the start's geometry.
	if ((ClaimedController != nullptr) &&
		(ClaimedController->GetPawn() != nullptr) &&
		(GetLocationOccupancy(ClaimedController) == EPlayerStartLocationOccupancy::Empty))
	{
		ClaimedController = nullptr;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
		}
	}
}
