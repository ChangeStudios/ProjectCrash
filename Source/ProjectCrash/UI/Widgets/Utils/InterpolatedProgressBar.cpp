// Copyright Samuel Reitich 2024.


#include "UI/Widgets/Utils/InterpolatedProgressBar.h"

UInterpolatedProgressBar::UInterpolatedProgressBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	CurrentInterpolationState(EInterpolationState::None),
	CurrentInterpolationData(),
	LastTimerTickTime(0.0f),
	EaseOutInterpolationExponent(1.5f),
	InterpolationUpdateInterval(0.0f)
{
}

float UInterpolatedProgressBar::GetTargetProgressValue() const
{
	return IsInterpolating() ? CurrentInterpolationData.TargetValue : GetPercent();
}

void UInterpolatedProgressBar::SetCurrentProgressValue(const float NewValue)
{
	// Set the progress bar's progress percentage, clamped between 0.0 and 1.0.
	SetPercent(FMath::Clamp(NewValue, 0.0f, 1.0f));
}

void UInterpolatedProgressBar::InterpolateToProgressValue(const float InTargetValue, float InMaximumInterpolationDuration, float InMinimumChangeRate, float InOutroOffset)
{
	const bool bWasTimerActive = TimerTickHandle.IsValid();

	// Cancel any ongoing interpolation.
	CancelInterpolation();

	// Validate the given parameters.
	ensureAlways(InMaximumInterpolationDuration > 0.0f);
	ensureAlways(InOutroOffset <= InMaximumInterpolationDuration);
	ensureAlways(InMinimumChangeRate >= 0.0f);

	InMaximumInterpolationDuration = FMath::Max(0.0f, InMaximumInterpolationDuration);
	InOutroOffset = FMath::Clamp(InOutroOffset, 0.0f, InMaximumInterpolationDuration);
	InMinimumChangeRate = FMath::Max(0.0f, InMinimumChangeRate);

	const float CurrentValue = GetPercent();

	// Don't perform any interpolation if we are already at the target value.
	if (InTargetValue == CurrentValue)
	{
		return;
	}

	// Enter the interpolation state.
	const float InterpolationDuration = CalculateInterpolationDuration(InTargetValue, InMinimumChangeRate, InMaximumInterpolationDuration);
	const float OutroOffset = FMath::Clamp(InOutroOffset, 0.0f, InterpolationDuration);
	EnterInterpolation(CurrentValue, InTargetValue, InterpolationDuration, OutroOffset);
	const bool bSuccessfullyStarted = CurrentInterpolationState != EInterpolationState::None;

	// Invoke the tick timer to begin interpolation.
	const UWorld* const World = GetWorld();
	if (World && bSuccessfullyStarted)
	{
		// Update the time of the last tick if the timer wasn't already active.
		if (!bWasTimerActive)
		{
			LastTimerTickTime = World->GetTimeSeconds();
		}

		// Start the tick timer. This callback will re-invoke itself with the timer until it finishes or is cancelled.
		TimerTickHandle = World->GetTimerManager().SetTimerForNextTick(this, &UInterpolatedProgressBar::OnTimerTick);
	}
}

bool UInterpolatedProgressBar::IsInterpolating() const
{
	// InterpolationState tracks when we are performing an interpolation.
	return CurrentInterpolationState == EInterpolationState::Interpolating;
}

float UInterpolatedProgressBar::CalculateInterpolationDuration(const float InTargetValue, const float InMinimumChangeRate, const float InMaximumInterpolationDuration) const
{
	// Cache the progress bar's current value.
	const float CurrentValue = GetPercent();

	// Calculate the minimum change rate, taking into account which direction we're interpolating.
	const float SignedMinimumChangeRate = InMinimumChangeRate * FMath::Sign(InTargetValue - CurrentValue);

	/* Calculate the interpolation duration that is less than or equal to the maximum specified duration and changes
	 * the progress value by at least the specified minimum rate. This formula only works if the minimum change rate is
	 * smaller than the total required change. */
	if (FMath::Abs(SignedMinimumChangeRate) < FMath::Abs(InTargetValue - CurrentValue))
	{
		/* This formula is just some math wizardry. See UCommonNumericTextBlock::CalculateInterpolationDuration for how
		 * it's derived. */
		const float DurationBasedOnMinimumChangeRate = 1.0f / (-FMath::Pow(-SignedMinimumChangeRate / (InTargetValue - CurrentValue) + 1.0f, 1.0f / EaseOutInterpolationExponent) + 1.0f);

		return FMath::Clamp(DurationBasedOnMinimumChangeRate, 0.0f, InMaximumInterpolationDuration);
	}
	/* If the formula cannot be used with the given parameters, estimate how long the interpolation will take, using
	 * the minimum change rate as the assumed change rate that will be used. */
	else
	{
		return FMath::Clamp((InTargetValue - CurrentValue) / SignedMinimumChangeRate, 0.0f, InMaximumInterpolationDuration);
	}
}

void UInterpolatedProgressBar::EnterInterpolation(const float InitialValue, const float FinalValue, const float Duration, const float OutroOffset)
{
	// Broadcast that this widget began an interpolation.
	InterpolationStartedDelegate.Broadcast(this);

	// Update our current interpolation state.
	CurrentInterpolationState = EInterpolationState::Interpolating;

	// Initialize the data used for the new interpolation.
	CurrentInterpolationData.ElapsedDuration = 0.0f;
	CurrentInterpolationData.TargetDuration = Duration;
	CurrentInterpolationData.SourceValue = InitialValue;
	CurrentInterpolationData.TargetValue = FinalValue;
	CurrentInterpolationData.OutroOffset = OutroOffset;
	CurrentInterpolationData.bHasTriggeredOutro = false;

	// Perform the initial tick.
	UpdateInterpolation();
}

void UInterpolatedProgressBar::UpdateInterpolation()
{
	// Cache the value before this update.
	const float LastValue = GetPercent();

	// Update the progress bar's current value (i.e. its "percent").
	const float Alpha = FMath::Clamp(CurrentInterpolationData.ElapsedDuration / CurrentInterpolationData.TargetDuration, 0.0f, 1.0f);
	SetPercent(FMath::InterpEaseOut(CurrentInterpolationData.SourceValue, CurrentInterpolationData.TargetValue, Alpha, EaseOutInterpolationExponent));

	// Broadcast that this widget performed an interpolation update.
	InterpolatedUpdatedDelegate.Broadcast(this, LastValue, GetPercent());

	// If the outro event has not yet been triggered, check if it should be.
	if (!CurrentInterpolationData.bHasTriggeredOutro)
	{
		// Trigger the outro event if we're at or past the outro point.
		if (CurrentInterpolationData.ElapsedDuration >= CurrentInterpolationData.TargetDuration - CurrentInterpolationData.OutroOffset)
		{
			CurrentInterpolationData.bHasTriggeredOutro = true;
			InterpolationOutroDelegate.Broadcast(this);
		}
	}

	// End the interpolation once its desired duration has elapsed.
	if (CurrentInterpolationData.ElapsedDuration >= CurrentInterpolationData.TargetDuration)
	{
		ExitInterpolation(true);
	}
}

void UInterpolatedProgressBar::ExitInterpolation(const bool bHasCompleted)
{
	// End our current interpolation state.
	CurrentInterpolationState = EInterpolationState::None;

	// Broadcast that this widget has finished its interpolation.
	InterpolationEndedDelegate.Broadcast(this, bHasCompleted);
}

void UInterpolatedProgressBar::CancelInterpolation()
{
	// Exit our current interpolation, specifying that we did not complete it.
	if (CurrentInterpolationState == EInterpolationState::Interpolating)
	{
		ExitInterpolation(false);
	}

	// Cancel the current timer if it is still active.
	if (CurrentInterpolationState == EInterpolationState::None && TimerTickHandle.IsValid())
	{
		if (const UWorld* const World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerTickHandle);
		}
	}
}

void UInterpolatedProgressBar::OnTimerTick()
{
	const UWorld* const World = GetWorld();

	if (!World)
	{
		return;
	}

	// Calculate the time since the last interpolation tick (not the time since the last world tick).
	const float CurrentTickTime = World->GetTimeSeconds();
	const float DeltaSeconds = CurrentTickTime - LastTimerTickTime;

	/* Since we can't actually perform updates at an interval of 0.0 seconds, we have to internally clamp the desired
	 * update interval if it's too low. We arbitrarily define 60 ticks-per-second as the fastest possible update
	 * rate. */
	const float MinimumUpdateInterval = 0.017f;
	const float UpdateRate = FMath::Max(InterpolationUpdateInterval, MinimumUpdateInterval);

	// Fix the interpolation updates to our desired rate.
	if (DeltaSeconds > UpdateRate)
	{
		// The Tick function performs the actual ticking logic.
		Tick(DeltaSeconds);

		/* Save the current time, so we know when the most recent interpolation tick was performed. This is what allows
		 * us to use a custom tick rate. */
		LastTimerTickTime = CurrentTickTime;

		// Cancel the timer if we've stopped interpolating.
		if (CurrentInterpolationState == EInterpolationState::None)
		{
			World->GetTimerManager().ClearTimer(TimerTickHandle);
			return;
		}
	}

	// If we haven't finished interpolating, continue ticking.
	TimerTickHandle = World->GetTimerManager().SetTimerForNextTick(this, &UInterpolatedProgressBar::OnTimerTick);
}

void UInterpolatedProgressBar::Tick(float DeltaSeconds)
{
	// Don't tick if we aren't interpolating.
	if (CurrentInterpolationState == EInterpolationState::None)
	{
		return;
	}

	// Don't tick when this widget is not in a valid widget tree, since it won't be rendered.
	if (!GetCachedWidget().IsValid())
	{
		return;
	}

	// Update the elapsed duration of the interpolation.
	const float TargetDuration = CurrentInterpolationData.TargetDuration;
	const float NewElapsedDuration = CurrentInterpolationData.ElapsedDuration + DeltaSeconds;

	/* If we accidentally interpolate past our target duration, cache how far over we've gone (so we can pass it onto
	 * the next interpolation, if possible) before clamping back to the target duration. */
	const float CurrentInterpolationOverflow = FMath::Max(0.0f, NewElapsedDuration - TargetDuration);
	CurrentInterpolationData.ElapsedDuration = FMath::Min(NewElapsedDuration, TargetDuration);

	// Perform an interpolation update.
	const EInterpolationState PreUpdateInterpolationState = CurrentInterpolationState;
	if (CurrentInterpolationState == EInterpolationState::Interpolating)
	{
		UpdateInterpolation();
	}

	/* If our interpolation was cancelled or interrupted during the update, start the next interpolation at our
	 * overflow point, if our last interpolation overflowed. */
	if (CurrentInterpolationState != PreUpdateInterpolationState)
	{
		CurrentInterpolationData.ElapsedDuration = CurrentInterpolationOverflow;
	}
}
