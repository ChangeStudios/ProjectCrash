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
 * Defines the behavior of spring models used for additive sway animations.
 */
USTRUCT(BlueprintType)
struct FFloatSpringInterpData
{
	GENERATED_BODY()

	/** Scalar used to scale the amplitude of the spring model and control the strength of its effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model")
	float InterpSpeed = 1.0f;

	/** Represents the stiffness of this spring. Higher values reduce oscillation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model")
	float Stiffness = 25.0f;

	/** The amount of damping applied to the spring. 0.0 means no damping (full oscillation), 1.0 means full damping
	 * (no oscillation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model")
	float CriticalDampingFactor = 0.5f;

	/** A multiplier that acts like mass on the spring, affecting the amount of force required to change its position. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spring Model")
	float Mass = 10.0f;
};



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

	/** First-person base pose. Additives are applied on top of this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Base Poses", DisplayName = "Base Pose (First-Person)")
	TObjectPtr<UAnimSequenceBase> BasePose_FPP = nullptr;

// Additive poses. These are applied onto base poses before animations.
public:

	/** First-person additive poses applied to offset the character's base pose based on their camera pitch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses", DisplayName = "Pitch Offset Poses")
	TObjectPtr<UBlendSpace1D> PitchOffsetBS_FPP = nullptr;

	/** First-person additive poses applied to offset the character's base pose based on their current turn-rate. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Aim Sway", DisplayName = "Aim Sway Offset Poses")
	TObjectPtr<UBlendSpace1D> AimSwayOffsetBS_FPP = nullptr;
	
	/** The upper and lower value used to normalize the pawn's current aim speed when it is used to apply additive aim
	 * sway animations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Aim Sway", DisplayName = "Maximum Aim Speed")
	float MaxAimSpeed = 500.0f;

	/** Data used to define the behavior of the spring used to control the up/down additive aim sway. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Aim Sway", DisplayName = "Aim Up/Down Spring Data")
	FFloatSpringInterpData AimSwayUpDownSpringData;

	/** Data used to define the behavior of the spring used to control the right/left additive aim sway. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Aim Sway", DisplayName = "Aim Right/Left Spring Data")
	FFloatSpringInterpData AimSwayRightLeftSpringData;

	/** First-person additive poses applied to offset the character's base pose based on their current movement speed and
	 * direction. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Move Sway", DisplayName = "Movement Sway Offset Poses")
	TObjectPtr<UBlendSpace> MoveSwayOffsetBS_FPP = nullptr;

	/** Data used to define the behavior of the spring used to control the forward/backward additive aim sway. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Move Sway", DisplayName = "Move Forward/Backkward Spring Data")
	FFloatSpringInterpData MoveSwayForwardBackwardSpringData;

	/** Data used to define the behavior of the spring used to control the right/left additive aim sway. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First-Person|Additive Poses|Move Sway", DisplayName = "Move Right/Left Spring Data")
	FFloatSpringInterpData MoveSwayRightLeftSpringData;

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

	/** Third-person base pose. Additives are applied on top of this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Base Poses", DisplayName = "Base Pose (Third-Person)")
	TObjectPtr<UAnimSequenceBase> BasePose_TPP = nullptr;

// Additive poses. These are applied onto base poses before animations.
public:

	/** Third-person additive poses applied to offset the character's base pose based on their camera pitch. This is
	 * how players' camera pitch is replicated, since their movement component only replicates their yaw. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Additive Poses", DisplayName = "Pitch Offset Poses")
	TObjectPtr<UBlendSpace1D> PitchOffsetBS_TPP = nullptr;

	/** The maximum normalized pitch to which the character's aim offset will be scaled. Setting this value to 1.0 will
	 * allow the character to look straight up (90 degrees). Setting it to any value less than 1.0 will clamp the pitch
	 * of their animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Additive Poses", DisplayName = "Maximum Normalized Pitch", Meta = (UIMax = 1, UIMin = -1))
	float MaxNormalizedPitch = 1.0f;

	/** The maximum normalized pitch to which the character's aim offset will be scaled. Setting this value to -1.0 will
	 * allow the character to look straight down (-90 degrees). Setting it to any value greater than -1.0 will clamp
	 * the pitch of their animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Additive Poses", DisplayName = "Minimum Normalized Pitch", Meta = (UIMax = 1, UIMin = -1))
	float MinNormalizedPitch = -1.0f;

// Locomotion.
public:

	/** Third-person idle animation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Idle")
	TObjectPtr<UAnimSequenceBase> Idle_TPP = nullptr;

	/** Third-person walking animations. This includes the idle animation when speed is 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Third-Person|Locomotion", DisplayName = "Walking")
	TObjectPtr<UBlendSpace> WalkBS_TPP = nullptr;

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
