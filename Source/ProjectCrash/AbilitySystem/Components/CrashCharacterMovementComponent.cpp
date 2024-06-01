// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "CrashAbilitySystemComponent.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "CrashGameplayTags.h"
#include "Characters/ChallengerBase.h"
#include "GameFramework/CrashLogging.h"

#define FALLING_TAG CrashGameplayTags::TAG_State_Movement_Falling
#define JUMPING_TAG CrashGameplayTags::TAG_State_Movement_Jumping

UCrashCharacterMovementComponent::UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
	GravityScale = 1.5;
	MaxAcceleration = 16384.0f;
	BrakingFriction = 4.0f;
	bUseSeparateBrakingFriction = true;

	GroundFriction = 0.0f;
	MaxWalkSpeed = 800.0f;
	MaxWalkSpeedCrouched = 400.0f;
	BrakingDecelerationWalking = 8192.0f;

	JumpZVelocity = 700.0f;
	AirControl = 0.75f;
	FallingLateralFriction = 0.4;
}

void UCrashCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACrashCharacterBase* OwningChar = Cast<ACrashCharacterBase>(GetOwner()))
	{
		// Bind the OnJumped callback to when this character jumps.
		OwningChar->JumpedDelegate.AddDynamic(this, &UCrashCharacterMovementComponent::OnJumped);
	}
}

void UCrashCharacterMovementComponent::OnJumped()
{
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		/* Apply the "jumping" gameplay tag when this character becomes airborne. It will be removed when the character
		 * lands. */
		CrashASC->AddLooseGameplayTag(JUMPING_TAG);

		// Jumping is a local event, so the server needs to be notified.
		CrashASC->AddReplicatedLooseGameplayTag(JUMPING_TAG);
	}
}

void UCrashCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// We don't want to use separate braking friction while airborne.
	bUseSeparateBrakingFriction = (MovementMode != MOVE_Falling);

	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		/* Apply the "falling" gameplay tag when this character becomes airborne. This is used to trigger the "falling"
		 * animation much more efficiently than with animation updates. */
		if (PreviousMovementMode != MOVE_Falling && MovementMode == MOVE_Falling)
		{
			CrashASC->SetLooseGameplayTagCount(FALLING_TAG, 1);
		}
		// Remove the "falling" and "jumping" tags when this character lands on the ground.
		else if (PreviousMovementMode == MOVE_Falling && MovementMode == MOVE_Walking)
		{
			CrashASC->SetLooseGameplayTagCount(FALLING_TAG, 0);
			CrashASC->SetLooseGameplayTagCount(JUMPING_TAG, 0);

			CrashASC->SetReplicatedLooseGameplayTagCount(JUMPING_TAG, 0);
		}
	}
}
