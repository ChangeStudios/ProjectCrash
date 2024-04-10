// Copyright Samuel Reitich 2024.


#include "UI/Framework/CrashActivatableWidget.h"

#include "Player/PlayerControllers/CrashPlayerController.h"

ACrashPlayerController* UCrashActivatableWidget::GetOwningCrashPlayer() const
{
	return Cast<ACrashPlayerController>(GetOwningPlayer());
}
