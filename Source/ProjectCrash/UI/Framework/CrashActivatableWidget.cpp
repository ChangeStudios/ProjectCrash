// Copyright Samuel Reitich 2024.


#include "UI/Framework/CrashActivatableWidget.h"

#include "Player/PlayerControllers/CrashPlayerControllerBase.h"

UCrashActivatableWidget::UCrashActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> UCrashActivatableWidget::GetDesiredInputConfig() const
{
	// Use the widget's input properties to determine its desired input configuration.
	switch (InputConfig)
	{
		case ECrashWidgetInputMode::GameAndMenu:
		{
			return FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
		}
		case ECrashWidgetInputMode::Game:
		{
			return FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
		}
		case ECrashWidgetInputMode::Menu:
		{
			return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
		}
		case ECrashWidgetInputMode::Default:
		default:
		{
			return TOptional<FUIInputConfig>();
		}
	}
}

ACrashPlayerControllerBase* UCrashActivatableWidget::GetOwningCrashPlayer() const
{
	return Cast<ACrashPlayerControllerBase>(GetOwningPlayer());
}
