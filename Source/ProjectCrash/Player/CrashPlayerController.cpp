// Copyright Samuel Reitich. All rights reserved.


#include "Player/CrashPlayerController.h"

#include "CrashPlayerState.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Camera/CrashPlayerCameraManager.h"

ACrashPlayerController::ACrashPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ACrashPlayerCameraManager::StaticClass();
}

void ACrashPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// Process ability input.
	if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent())
	{
		CrashASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

ACrashPlayerState* ACrashPlayerController::GetCrashPlayerState() const
{
	return CastChecked<ACrashPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UCrashAbilitySystemComponent* ACrashPlayerController::GetCrashAbilitySystemComponent() const
{
	const ACrashPlayerState* CrashPS = GetCrashPlayerState();
	return (CrashPS ? CrashPS->GetCrashAbilitySystemComponent() : nullptr);
}
