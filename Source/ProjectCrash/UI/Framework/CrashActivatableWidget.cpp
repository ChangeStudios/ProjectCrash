// Copyright Samuel Reitich 2024.


#include "UI/Framework/CrashActivatableWidget.h"

#include "CommonInputSubsystem.h"
#include "Animation/WidgetAnimation.h"
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

void UCrashActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// Play an optional activation animation. 
	if (OnActivated)
	{
		PlayAnimationForward(OnActivated);
	}

	// Set the default focus target if GetDesiredFocusTarget has been overridden.
	if (UWidget* FocusTarget = GetDesiredFocusTarget())
	{
		FocusTarget->SetFocus();

		// Each time input method changes, if it changes to a gamepad, reset focus to the default focus target.
		if (UCommonInputSubsystem* CommonInput = GetOwningLocalPlayer()->GetSubsystem<UCommonInputSubsystem>())
		{
			CommonInput->OnInputMethodChangedNative.AddWeakLambda(this, [FocusTarget] (ECommonInputType NewInputType)
			{
				FocusTarget->SetFocus();
			});
		}
	}
}

ACrashPlayerControllerBase* UCrashActivatableWidget::GetOwningCrashPlayer() const
{
	return Cast<ACrashPlayerControllerBase>(GetOwningPlayer());
}
