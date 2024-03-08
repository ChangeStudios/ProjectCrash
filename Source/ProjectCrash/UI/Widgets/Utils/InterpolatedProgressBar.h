// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "InterpolatedProgressBar.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterpolationStartedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInterpolationUpdatedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterpolationOutroSignature, UInterpolatedProgressBar*, InterpolatedProgressBar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterpolationEndedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar, const bool, bHasCompleted);

/**
 * A progress bar which supports interpolation of its displayed value.
 */
UCLASS()
class PROJECTCRASH_API UInterpolatedProgressBar : public UProgressBar
{
	GENERATED_BODY()

public:

	UInterpolatedProgressBar(const FObjectInitializer& ObjectInitializer);

	virtual void SynchronizeProperties() override;

	/** Returns the progress bar value this widget will ultimately display if it is interpolating, or the current value
	 * if it is not. Normalized from 0.0 to 1.0. */
	UFUNCTION(BlueprintCallable, Category = "Interpolated Progress Bar")
	float GetTargetProgressValue() const;

	/** Sets the current progress bar value. Cancels any ongoing interpolation. NewValue is clamped between 0.0 and
	 * 1.0. */
	UFUNCTION(BlueprintCallable, Category = "Interpolated Progress Bar")
	float SetCurrentProgressValue(const float NewValue) const;

	/**
	 * Starts an ongoing process of interpolating the current progress bar value to the specified target value. The
	 * interpolation process may take MaximumInterpolationDuration, or complete sooner if the value of
	 * MinimumRechargeRate causes the target to be reached prematurely. An outro duration can optionally be specified
	 * in order to trigger an outro event before interpolation completes.
	 *
	 * @param TargetValue					The progress bar value to which to interpolate.
	 * @param MaximumInterpolationDuration	The duration, in seconds, for the interpolation to take, at most. Must be
	 *										greater than 0.
	 * @param MinimumRechargeRate			The minimum change in the progress bar's value per second. Must be greater
	 *										than or equal to 0.
	 * @param OutroOffset					The amount of time, in seconds, before the end of the interpolation at
	 *										which to trigger the outro event. Must be less than or equal to
	 *										MaximumInterpolationDuration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Progress Bar Interpolation")
	void InterpolateToProgressValue(const float TargetValue, float MaximumInterpolationDuration = 3.0f, float MinimumRechargeRate = 1.0f, float OutroOffset = 0.0f);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Bar Interpolation")
	bool IsInterpolatingProgressValue() const;

	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Started")
	FInterpolationStartedSignature InterpolationStartedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Updated")
	FInterpolationUpdatedSignature InterpolatedUpdatedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Outro Event")
	FInterpolationOutroSignature InterpolationOutroDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Ended")
	FInterpolationEndedSignature InterpolationEndedDelegate;

	/** Exponent parameter for the "ease out" interpolation curve. Must be > 0, but should be > 1 in order to "ease out". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numeric Interpolation", meta = (ClampMin = "1.0"))
	float EaseOutInterpolationExponent;

	/** The desired interval, in seconds, between interpolation updates. 0.0 implies per-frame updates. NOTE: Interpolation updates may occur further apart due to tick rates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Numeric Interpolation", meta = (ClampMin = "0.0"))
	float InterpolationUpdateInterval;

protected:

	void UpdateDisplayedProgress();

private:

	float CalculateInterpolationDuration(const float InTargetValue, const float InMinimumChangeRate, const float InMaximumInterpolationDuration) const;

	void OnTimerTick();
	void Tick(float DeltaTime);

	void EnterProgressInterpolation(const float InitialValue, const float FinalValue, const float Duration, const float OutroOffset);
	void UpdateProgressInterpolation();
	void ExitProgressInterpolation(const bool bHasCompleted = false);

	void CancelInterpolation();

private:

	FTimerHandle TimerTickHandle;
	float LastTimerTickTime;

	struct
	{
		float ElapsedStateDuration;
	} InterpolationState;

	struct
	{
		float SourceValue;
		float TargetValue;
		float OutroOffset;
		bool bHasTriggeredOutro;
		float Duration;
	} ProgressInterpolationState;
};
