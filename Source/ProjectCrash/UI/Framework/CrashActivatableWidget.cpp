// Copyright Samuel Reitich 2024.


#include "UI/Framework/CrashActivatableWidget.h"

#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

ACrashPlayerControllerBase* UCrashActivatableWidget::GetOwningCrashPlayer() const
{
	return Cast<ACrashPlayerControllerBase>(GetOwningPlayer());
}
