// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityTask_PlayDualMontageAndWait.generated.h"

/**
 * Extends the PlayMontageAndWait task to play two montages: one on the character's first-person mesh and one on the
 * third-person mesh.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_PlayDualMontageAndWait : public UAbilityTask
{
	GENERATED_BODY()

	// Task output pins.

public:

	/** Called when this task was successfully completed, i.e. the third-person montage successfully finished
	 * playing AND finished blending out. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnCompleted;

	/** Called when the third-person montage begins blending out. If it blends out after being interrupted,
	 * OnInterrupted will be called instead of this. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnBlendOut;

	/** Called if the third-person montage is interrupted by another montage before it finishes playing. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnInterrupted;

	/** Called if this task is cancelled, which stops any montages that are currently playing. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnCancelled;



	// Task callbacks.

public:

	/** Callback function for when the third-person montage finishes blending out. */
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	/** Callback function for when the owning Gameplay Ability is cancelled. Stops playing this task's montages and
	 * ends this task. */
	UFUNCTION()
	void OnGameplayAbilityCancelled();

	/** Callback function for when the third-person montage finishes. */
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Broadcasts the OnCancelled delegate when this task is externally cancelled. */
	virtual void ExternalCancel() override;

protected:

	/** Delegate fired when the third-person montage begins blending out. If the third-person montage was not played,
	 * this is never fired. */
	FOnMontageBlendingOutStarted BlendingOutDelegate;

	/** Delegate fired when the third-person montage finishes playing. If the third-person montage was not played,
	 * this is never fired. */
	FOnMontageEnded MontageEndedDelegate;

	/** Delegate fired when another montage is played on the third-person mesh before the third-person montage is
	 * finished playing. If the third-person montage was not played, this is never fired. */
	FDelegateHandle InterruptedHandle;

	/** Stops ongoing montages when the owning ability ends if bStopWhenAbilityEnds is true. */
	virtual void OnDestroy(bool AbilityEnded) override;



	// Ability task.

public:

	/** 
	 * Plays an animation montage on the avatar actor's first-person mesh and an animation montage on the third-person
	 * mesh. The avatar actor must be a child of AChallengerCharacterBase and have a valid third-person mesh to play
	 * either montage. Otherwise, the task immediately ends without doing anything.
	 *
	 * This task's ability logic uses the third-person montage, meaning the first-person montage does not need to be
	 * played for this task to function correctly (e.g. NPCs don't need first-person montages). If both montages are
	 * played, ensure that they are the same length for the best results.
	 *
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped
	 * when the ability is explicitly cancelled.
	 *
	 * On normal execution, OnBlendOut is called when the third-person montage begins blending out, and OnCompleted
	 * when it is completely done playing. OnInterrupted is called if another montage overwrites the third-person
	 * montage (e.g. if PlayMontageAndWait is called), and OnCancelled is called if the ability or task is cancelled.
	 *
	 * @param TaskInstanceName					Overrides the name of this task for later querying.
	 * @param FirstPersonMontageToPlay			Montage to play on the character's first-person mesh.
	 * @param ThirdPersonMontageToPlay			Montage to play on the character's third-person mesh.
	 * @param FirstPersonRate					Rate at which the first-person montage is played.
	 * @param ThirdPersonRate					Rate at which the third-person montage is played.
	 * @param FirstPersonStartSection			If not empty, named montage section from which to start the 
	 *											first-person montage.
	 * @param ThirdPersonStartSection			If not empty, named montage section from which to start the 
	 *											third-person montage.
	 * @param bStopWhenAbilityEnds				If true, this montage will be aborted if the ability ends normally. It
	 *											is always stopped when the ability is explicitly cancelled.
	 * @param AnimRootMotionTranslationScale	Modifies the size of root motion. Set to 0 to block it entirely.
	 * @param FirstPersonStartTimeSeconds		Starting time offset in the first-person montage. This will be
	 *											overridden by FirstPersonStartSection if that is also set.
	 * @param ThirdPersonStartTimeSeconds		Starting time offset in the third-person montage. This will be
	 *											overridden by ThirdPersonStartSection if that is also set.
	 * @param bAllowInterruptAfterBlendOut		If true, you can receive OnInterrupted after an OnBlendOut started.
	 *											Otherwise, OnInterrupted will not fire when interrupted, but you will
	 *											not get OnComplete.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", Meta = (DisplayName = "PlayDualMontageAndWait",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayDualMontageAndWait* CreatePlayDualMontageAndWaitProxy
	(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* FirstPersonMontageToPlay,
		UAnimMontage* ThirdPersonMontageToPlay,
		float FirstPersonRate = 1.0f,
		float ThirdPersonRate = 1.0f,
		FName FirstPersonStartSection = NAME_None,
		FName ThirdPersonStartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.0f,
		float FirstPersonStartTimeSeconds = 0.0f,
		float ThirdPersonStartTimeSeconds = 0.0f,
		bool bAllowInterruptAfterBlendOut = false
	);

	/** Begins ability task logic. */
	virtual void Activate() override;



	// Task parameters.

protected:

	UPROPERTY()
	TObjectPtr<UAnimMontage> FirstPersonMontageToPlay;

	UPROPERTY()
	TObjectPtr<UAnimMontage> ThirdPersonMontageToPlay;

	UPROPERTY()
	float FirstPersonRate;

	UPROPERTY()
	float ThirdPersonRate;

	UPROPERTY()
	FName FirstPersonStartSection;

	UPROPERTY()
	FName ThirdPersonStartSection;

	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	float AnimRootMotionTranslationScale;

	UPROPERTY()
	float FirstPersonStartTimeSeconds;

	UPROPERTY()
	float ThirdPersonStartTimeSeconds;

	UPROPERTY()
	bool bAllowInterruptAfterBlendOut;



	// Utils.

protected:

	/** Checks if the ability is playing a montage and stops that montage. Returns whether a montage was stopped. */
	bool StopPlayingMontage();

	/** Returns a string identifying this task. */
	virtual FString GetDebugString() const override;
};
