// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilityTask_PlayDualMontageAndWait.generated.h"

/**
 * Plays two montages: one on the avatar's first-person mesh and one on their third-person mesh. Avatar must be of type
 * CrashCharacter in order to access both meshes. To only play a third-person montage, use PlayMontageAndWait.
 *
 * The third-person montage acts as the "authoritative" montage. I.e. playback events are triggered by the third-person
 * montage. This prevents conflicting events, such as if one montage is interrupted, while another finishes
 * successfully. This means the montages don't have to be the same length (though they should be).
 *
 * Currently, this task can playing both montages or just the third-person montage. If we wanted to, we could easily
 * refactor this task to also be able to play just the first-person montage, if we wanted to. We would just have to
 * track the "authoritative" montage, instead of always using the third-person montage.
 *
 * TODO: Subclass this into PlayDualMontageAndWaitForEvent.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_PlayDualMontageAndWait : public UAbilityTask
{
	GENERATED_BODY()

	// Task output pins.

public:

	/** Fired when the third-person montage BEGINS blending out. If a montage begins blending out because it was
	 * interrupted, OnInterrupted will be fired instead of this. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnBlendOut;

	/** Called when the third-person montage finishes playing and finishes blending out. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnCompleted;

	/** Fired if the third-person montage is interrupted by another montage before it finishes playing. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnInterrupted;

	/** Fired if this task is cancelled. Stops any montages that are currently playing. Does not trigger OnBlendOut or
	 * OnInterrupted. */
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate OnCancelled;



	// Task callbacks.

public:

	/** Callback function for when the third-person montage begins blending out. */
	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	/** Callback function for when the third-person montage finishes playing and finishes blending out. */
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** Callback function for when the owning gameplay ability is cancelled. Ends this task and stops playing any
	 * montages, if requested. */
	UFUNCTION()
	virtual void OnGameplayAbilityCancelled();

	/** Broadcasts OnCancelled when this task is externally cancelled. */
	virtual void ExternalCancel() override;

// Internal delegates.
protected:

	/** Fired when the third-person montage begins blending out. */
	FOnMontageBlendingOutStarted BlendingOutDelegate;

	/** Fired when the third-person montage finishes playing and finishes blending out. */
	FOnMontageEnded MontageEndedDelegate;

	/** Fired if another montage is played on the third-person mesh before the third-person montage finishes playing. */
	FDelegateHandle InterruptedHandle;

	/** Stops ongoing montages when the owning ability ends, if bStopWhenAbilityEnds is true. */
	virtual void OnDestroy(bool AbilityEnded) override;



	// Task construction.

public:

	/**
	 * Plays an animation montage on the avatar actor's first-person mesh and third-person mesh. The avatar must be of
	 * type CrashCharacter.
	 *
	 * The third-person montage is considered the "authority" for this task; output pins are fired in response to the
	 * third-person montage's playback events. This means that both montages do not need to be the same length. It also
	 * means that this task CAN be used without a first-person mesh or without a first-person montage, but must always
	 * have a third-person montage.
	 *
	 * To cover all cases (e.g. guaranteeing logic executes when this task ends), hook into OnInterrupted, OnCancelled,
	 * and OnBlendOut OR OnCompleted. Hooking the same logic into both OnBlendOut and OnCompleted will run that logic
	 * twice: once when the montage finishes playing and once when it finishes blending out.
	 *
	 * @param TaskInstanceName					Overrides the name of this task for later querying.
	 * @param FirstPersonMontageToPlay			Montage to play on the character's first-person mesh.
	 * @param ThirdPersonMontageToPlay			Montage to play on the character's third-person mesh.
	 * @param FirstPersonRate					Rate at which the first-person montage is played.
	 * @param ThirdPersonRate					Rate at which the third-person montage is played.
	 * @param FirstPersonStartSection			(Optional) Name of the montage section from which to start the
	 *											first-person montage.
	 * @param ThirdPersonStartSection			(Optional) Name of the Montage section from which to start the 
	 *											third-person montage.
	 * @param bStopWhenAbilityEnds				If true, the montages will be cancelled if the ability ends normally.
	 *											Montages are always stopped when the ability is explicitly cancelled,
	 *											regardless of this value. Note that this task's output pins will not
	 *											fire after this task ends, even if this value is false.
	 * @param AnimRootMotionTranslationScale	Modifies the size of root motion. Set to 0 to block it entirely.
	 * @param FirstPersonStartTimeSeconds		Playback time at which to start the first-person montage. This will be
	 *											overridden by FirstPersonStartSection if that is also set.
	 * @param ThirdPersonStartTimeSeconds		Playback time at which to start the third-person montage. This will be
	 *											overridden by ThirdPersonStartSection if that is also set.
	 * @param bAllowInterruptAfterBlendOut		If true, OnInterrupted can be fired AFTER the montage begins blending
	 *											out, meaning both OnBlendOut and OnInterrupted can BOTH be fired. If
	 *											false, OnInterrupted will not fire if the montage is interrupted after
	 *											it already began blending out.
	 *											TODO: OnComplete does not fire when the montage is interrupted if this is false.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", Meta = (DisplayName = "PlayDualMontageAndWait",
		HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
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

	/** Stops any montages that the outer ability is currently playing. Returns whether a montage was stopped. */
	bool StopPlayingMontage();

	/** Returns a string identifying this task. */
	virtual FString GetDebugString() const override;
};
