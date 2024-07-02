// Copyright Samuel Reitich. All rights reserved.


#include "Camera/CrashPlayerCameraManager.h"

#include "CrashCameraComponent.h"
#include "Engine/Canvas.h"

void ACrashPlayerCameraManager::GetCameraBlendInfo(float& OutTopCameraWeight, FGameplayTag& OutTopCameraTag) const
{
	// Retrieve the local player's pawn. Can be a spectator.
	const APawn* Pawn = (PCOwner ? PCOwner->GetPawnOrSpectator() : nullptr);

	// Get the local pawn's camera component information.
	if (const UCrashCameraComponent* CameraComponent = UCrashCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->GetBlendInfo(OutTopCameraWeight, OutTopCameraTag);
	}
}

void ACrashPlayerCameraManager::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

	DisplayDebugManager.SetFont(GEngine->GetSmallFont());
	DisplayDebugManager.SetDrawColor(FColor::Yellow);
	DisplayDebugManager.DrawString(FString::Printf(TEXT("CrashPlayerCameraManager: %s"), *GetNameSafe(this)));

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	const APawn* Pawn = (PCOwner ? PCOwner->GetPawnOrSpectator() : nullptr);

	if (const UCrashCameraComponent* CameraComponent = UCrashCameraComponent::FindCameraComponent(Pawn))
	{
		CameraComponent->DrawDebug(Canvas);
	}
}