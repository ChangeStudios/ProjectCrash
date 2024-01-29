// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAnimData.generated.h"

class UAnimMontage;
class UAnimSequenceBase;
class UBlendSpace;
class UBlendSpace1D;

/**
 * Data that defines character animations. This is extended for more specific animation states (e.g. when an equipment
 * set is equipped).
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API UCharacterAnimData : public UDataAsset
{
	GENERATED_BODY()

	// First-person animations.

// Base poses. Additive animations are applied on top of these.
public:

	/** First-person base pose when standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Base Poses", DisplayName = "Standing Pose")
	TObjectPtr<UAnimSequenceBase> StandingPose_FPP = nullptr;

// Additive poses. These are applied onto base poses before animations.
public:

	/** First-person additive poses applied to offset the character's base pose based on their camera pitch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses", DisplayName = "Pitch Offset Poses")
	TObjectPtr<UBlendSpace1D> PitchOffsetBS_FPP = nullptr;

	/** First-person additive poses applied to offset the character's base pose based on their current turn-rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses", DisplayName = "Aim Sway Offset Poses")
	TObjectPtr<UBlendSpace1D> AimSwayOffsetBS_FPP = nullptr;

	/** First-person additive poses applied to offset the character's base pose based on their current movement speed and
	 * direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses", DisplayName = "Movement Sway Offset Poses")
	TObjectPtr<UBlendSpace> MoveSwayOffsetBS_FPP = nullptr;

// Locomotion.
public:

	/** First-person idle animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Locomotion", DisplayName = "Idle")
	TObjectPtr<UAnimSequenceBase> Idle_FPP = nullptr;

	/** First-person walking animations. This includes the idle animation when speed is 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Locomotion", DisplayName = "Walking")
	TObjectPtr<UBlendSpace1D> WalkBS_FPP = nullptr;

	/** First-person jump animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Locomotion", DisplayName = "Jump")
	TObjectPtr<UAnimSequenceBase> Jump_FPP = nullptr;

	/** First-person falling animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Locomotion", DisplayName = "Falling")
	TObjectPtr<UAnimSequenceBase> Falling_FPP = nullptr;

	/** First-person landing animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Locomotion", DisplayName = "Land")
	TObjectPtr<UAnimSequenceBase> Landing_FPP = nullptr;



	// Third-person animations.

// Base poses. Additive animations are applied on top of these.
public:

	/** Third-person base pose when standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Base Poses", DisplayName = "Standing Pose")
	TObjectPtr<UAnimSequenceBase> StandingPose_TPP = nullptr;

// Additive poses. These are applied onto base poses before animations.
public:

	/** Third-person additive poses applied to offset the character's base pose based on their camera pitch. This is
	 * how players' camera pitch is replicated, since their movement component only replicates their yaw. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Additive Poses", DisplayName = "Pitch Offset Poses")
	TObjectPtr<UBlendSpace1D> PitchOffsetBS_TPP = nullptr;

// Locomotion.
public:

	/** Third-person idle animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Idle")
	TObjectPtr<UAnimSequenceBase> Idle_TPP = nullptr;

	/** Third-person walking animations. This includes the idle animation when speed is 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Walking")
	TObjectPtr<UBlendSpace1D> WalkBS_TPP = nullptr;

	/** Third-person jump animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Jump")
	TObjectPtr<UAnimSequenceBase> Jump_TPP = nullptr;

	/** Third-person falling animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Falling")
	TObjectPtr<UAnimSequenceBase> Falling_TPP = nullptr;

	/** Third-person landing animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Land")
	TObjectPtr<UAnimSequenceBase> Landing_TPP = nullptr;
};
