// Copyright Samuel Reitich. All rights reserved.


#include "UI/CrashActivatableWidget.h"

#include "CommonInputSubsystem.h"
#include "Player/CrashPlayerController.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "UserInterface"

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
		default:
		{
			return TOptional<FUIInputConfig>();
		}
	}
}

void UCrashActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// If an "OnActivated" animation exists, play it when this widget is activated.
	if (OnActivated)
	{
		PlayAnimationForward(OnActivated);
	}

	// // Reset focus when this widget is activated.
	// if (UWidget* FocusTarget = GetDesiredFocusTarget())
	// {
	// 	FocusTarget->SetFocus();
	//
	// 	// Each time input method changes, if it changes to a gamepad, reset focus to the default focus target.
	// 	if (UCommonInputSubsystem* CommonInput = GetOwningLocalPlayer()->GetSubsystem<UCommonInputSubsystem>())
	// 	{
	// 		CommonInput->OnInputMethodChangedNative.AddWeakLambda(this, [FocusTarget] (ECommonInputType NewInputType)
	// 		{
	// 			FocusTarget->SetFocus();
	// 		});
	// 	}
	// }
}

ACrashPlayerController* UCrashActivatableWidget::GetOwningCrashPlayer() const
{
	return (IsValid(GetOwningPlayer()) ? Cast<ACrashPlayerController>(GetOwningPlayer()) : nullptr);
}

#if WITH_EDITOR
void UCrashActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UCrashActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		// Warn if GetDesiredFocusTarget is not implemented for this widget.
		if (GetParentNativeClass(GetClass()) == UCrashActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget is not implemented: gamepads will have trouble using this widget. If this widget does not require navigation, consider using UserWidget. You may implement GetDesiredFocusTarget and return null to suppress this warning."));
		}
		/* If this widget's direct parent is not CrashActivatableWidget, GetDesiredFocusTarget might be implemented in
		 * the child of CrashActivatableWidget from which this widget derives. */
		else
		{
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget is not implemented: gamepads will have trouble using this widget. This message can be safely ignored if GetDesiredFocusTrget is implemented in a parent of this widget."));
		}
	}
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE