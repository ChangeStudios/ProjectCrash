// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ProgressBar.h"
#include "InterpolatedProgressBar.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterpolationStartedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FInterpolationUpdatedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInterpolationOutroSignature, UInterpolatedProgressBar*, InterpolatedProgressBar);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInterpolationEndedSignature, UInterpolatedProgressBar*, InterpolatedProgressBar, const bool, bHasCompleted);

/**
 * A progress bar which supports interpolation of its displayed progress value.
 */
UCLASS()
class PROJECTCRASH_API UInterpolatedProgressBar : public UProgressBar
{
	GENERATED_BODY()

	// Construction.

public:

    /** Default constructor. */
	UInterpolatedProgressBar(const FObjectInitializer& ObjectInitializer);



	// Interpolation.
	
public:

	/** Returns the progress bar value this widget will ultimately display if it is interpolating, or the current value
	 * if it is not. */
	UFUNCTION(BlueprintCallable, Category = "Interpolated Progress Bar")
	float GetTargetProgressValue() const;

	/** Sets the current progress bar value. Cancels any ongoing interpolation. NewValue is clamped between 0.0 and
	 * 1.0. */
	UFUNCTION(BlueprintCallable, Category = "Interpolated Progress Bar")
	void SetCurrentProgressValue(const float NewValue);

	/**
	 * Starts an ongoing process of interpolating the current progress bar value to the specified target value. The
	 * interpolation process may take MaximumInterpolationDuration, or complete sooner if the value of
	 * MinimumRechargeRate causes the target to be reached prematurely. An outro duration can optionally be specified
	 * in order to trigger an outro event before interpolation completes.
	 *
	 * @param TargetValue					The progress bar value to which to interpolate.
	 * @param MaximumInterpolationDuration	The duration, in seconds, for the interpolation to take, at most. Must be
	 *										greater than 0.
	 * @param MinimumChangeRate				The minimum change in the progress bar's value per second. Must be greater
	 *										than or equal to 0.
	 * @param OutroOffset					The amount of time, in seconds, before the end of the interpolation at
	 *										which to trigger the outro event. Must be less than or equal to
	 *										MaximumInterpolationDuration.
	 */
	UFUNCTION(BlueprintCallable, Category = "Progress Bar Interpolation")
	void InterpolateToProgressValue(const float TargetValue, float MaximumInterpolationDuration = 3.0f, float MinimumChangeRate = 1.0f, float OutroOffset = 0.0f);

	/** Whether this widget is currently interpolating its progress value. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Progress Bar Interpolation")
	bool IsInterpolating() const;

// Internals.
private:

	/** Calculates the duration of this widget's interpolation with the given parameters. */
	float CalculateInterpolationDuration(const float InTargetValue, const float InMinimumChangeRate, const float InMaximumInterpolationDuration) const;

	/** Starts a new interpolation for this widget with the given parameters. */
	void EnterInterpolation(const float InitialValue, const float FinalValue, const float Duration, const float OutroOffset);

	/** Ticks this widget's ongoing interpolation, if it exists. */
	void UpdateInterpolation();

	/** Ends this widget's current interpolation, regardless of if it has finished or not. */
	void ExitInterpolation(const bool bHasCompleted = false);

	/** Cancels any ongoing interpolation, stopping this widget's progress at its current value. */
	void CancelInterpolation();

	/** Tracks whether we are currently interpolating this widget's progress. */
	enum class EInterpolationState
	{
		None,
		Interpolating
	} CurrentInterpolationState;

	/** Tracks data for the ongoing interpolation state. */
	struct
	{
		float ElapsedDuration;
		float TargetDuration;
		float SourceValue;
		float TargetValue;
		float OutroOffset;
		bool bHasTriggeredOutro;
	} CurrentInterpolationData;

// Timing.
private:

	/** Primary tick function, called each tick when relevant (i.e. when an interpolation is in progress). Used as a
	 * wrapper for Tick to implement a fixed update rate. */
	void OnTimerTick();

	/** Tick function called at a fixed rate. Invoked by OnTimerTick. */
	void Tick(float DeltaSeconds);

	/** Timer for invoking tick functions. */
	FTimerHandle TimerTickHandle;

	/** Tracks how long it's been since we performed a Tick update (not how long it's been between world ticks). Used
	 * to implement fixed tick rates. */
	float LastTimerTickTime;



	// Interpolation event delegates.

public:

	/** Fired when an interpolation begins. */
	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Started")
	FInterpolationStartedSignature InterpolationStartedDelegate;

	/** Fired each interpolation update (each tick through an interpolation). */
	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Updated")
	FInterpolationUpdatedSignature InterpolatedUpdatedDelegate;

	/** Fired when the interpolation outro begins. This can occur before the interpolation ends if desired. */
	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Outro Event")
	FInterpolationOutroSignature InterpolationOutroDelegate;

	/** Fired when the interpolation completely ends. This is fired whether the interpolation successfully completed
	 * or if it was ended prematurely. */
	UPROPERTY(BlueprintAssignable, Category = "Progress Bar Interpolation", DisplayName = "On Interpolation Ended")
	FInterpolationEndedSignature InterpolationEndedDelegate;



	// Interpolation parameters.

public:

	/** Exponent parameter for the "ease out" interpolation curve. Must be > 1.0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress Bar Interpolation", meta = (ClampMin = "1.0"))
	float EaseOutInterpolationExponent;

	/** The desired interval, in seconds, between interpolation updates. 0.0 implies per-frame updates. Note that
	 * interpolation updates may occur further apart than this due to tick rates. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress Bar Interpolation", meta = (ClampMin = "0.0"))
	float InterpolationUpdateInterval;
};
