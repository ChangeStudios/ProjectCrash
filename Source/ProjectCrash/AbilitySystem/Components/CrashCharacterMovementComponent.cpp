// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "CrashAbilitySystemComponent.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Characters/ChallengerBase.h"

#define FALLING_TAG CrashGameplayTags::TAG_State_Movement_Falling
#define JUMPING_TAG CrashGameplayTags::TAG_State_Movement_Jumping

UCrashCharacterMovementComponent::UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
}

void UCrashCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACrashCharacterBase* OwningChar = Cast<ACrashCharacterBase>(GetOwner()))
	{
		// Bind the OnJumped callback to when this character jumps.
		OwningChar->JumpedDelegate.AddDynamic(this, &UCrashCharacterMovementComponent::OnJumped);

		// Bind the OnLanded callback to when this character lands.
		OwningChar->LandedDelegate.AddDynamic(this, &UCrashCharacterMovementComponent::OnLanded);
	}
}

void UCrashCharacterMovementComponent::OnJumped()
{
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		/* Apply the "jumping" gameplay tag when this character becomes airborne. It will be removed when the character
		 * lands. */
		CrashASC->AddLooseGameplayTag(JUMPING_TAG);
	}
}

void UCrashCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		/* Apply the "falling" gameplay tag when this character becomes airborne. This is used to trigger the "falling"
		 * animation much more efficiently than with animation updates. */
		if (PreviousMovementMode != MOVE_Falling && MovementMode == MOVE_Falling)
		{
			CrashASC->AddLooseGameplayTag(FALLING_TAG);
		}
	}
}

void UCrashCharacterMovementComponent::OnLanded(const FHitResult& Hit)
{
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		// Remove all "Jumping" and "Falling" tags when the character lands.
		CrashASC->SetLooseGameplayTagCount(JUMPING_TAG, 0);
		CrashASC->SetLooseGameplayTagCount(FALLING_TAG,0);
	}
}
