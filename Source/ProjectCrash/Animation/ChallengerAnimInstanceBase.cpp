// Copyright Samuel Reitich. All rights reserved.


#include "Animation/ChallengerAnimInstanceBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AnimData/CharacterAnimData.h"
#include "Camera/CameraComponent.h"
#include "Characters/CrashCharacter.h"
#include "Characters/PawnExtensionComponent.h"
#include "Characters/Data/ChallengerSkinData.h"
#include "GameFramework/CrashLogging.h"

void UChallengerAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Enable multi-threading for updating animations.
	bUseMultiThreadedAnimationUpdate = true;
}

void UChallengerAnimInstanceBase::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	// Cache the owning challenger character.
	OwningCharacter = TryGetPawnOwner() ? Cast<ACrashCharacter>(TryGetPawnOwner()) : nullptr;

	// This animation instance won't work without a valid Challenger.
	if (!IsValid(OwningCharacter))
	{
		ANIMATION_LOG(Fatal, TEXT("OwningChallenger not valid in animation instance [%s] for [%s]. The animation system will not work properly."), *GetName(), *GetNameSafe(TryGetPawnOwner()));
		bUseMultiThreadedAnimationUpdate = false;
		return;
	}

	// Listen for when the owning character's ASC is initialized.
	UPawnExtensionComponent::FindPawnExtensionComponent(OwningCharacter)->OnAbilitySystemInitialized_RegisterAndCall(FSimpleDelegate::CreateUObject(this, &UChallengerAnimInstanceBase::OnASCInitialized));
}

bool UChallengerAnimInstanceBase::ThreadSafeHasTagExact(UAbilitySystemComponent* ASC, FGameplayTag TagToSearch) const
{
	if (!IsValid(ASC))
	{
		return false;
	}

	return ASC->GetGameplayTagCount(TagToSearch) > 0;
}

int UChallengerAnimInstanceBase::ThreadSafeTagCount(UAbilitySystemComponent* ASC, FGameplayTag TagToCount) const
{
	if (!IsValid(ASC))
	{
		return false;
	}

	return ASC->GetGameplayTagCount(TagToCount);
}

void UChallengerAnimInstanceBase::OnASCInitialized()
{
	ANIMATION_LOG(VeryVerbose, TEXT("ASC Initialized on [%s] for [%s] owned by [%s]"), *GetClientServerContextString(OwningCharacter), *GetName(), *GetNameSafe(GetOwningActor()));

	if (OwningASC)
	{
		ANIMATION_LOG(Warning, TEXT("Attempted to initialize ASC with [%s] when one already exists."), *GetName());
		return;
	}

	OwningASC = OwningCharacter->GetCrashAbilitySystemComponent();
}

void UChallengerAnimInstanceBase::UpdateAnimData(UCharacterAnimData* NewAnimData, UEquipmentSetSkinData* EquipmentSetSkinData)
{
	if (NewAnimData)
	{
		CurrentAnimationData = NewAnimData;

		if (EquipmentSetSkinData)
		{
			if (EquipmentSetSkinData->BasePose_FPP)
			{
				CurrentAnimationData->BasePose_FPP = EquipmentSetSkinData->BasePose_FPP;
			}

			if (EquipmentSetSkinData->BasePose_TPP)
			{
				CurrentAnimationData->BasePose_TPP = EquipmentSetSkinData->BasePose_TPP;
			}
		}
	}
}

void UChallengerAnimInstanceBase::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateMovementVelocity();
}

void UChallengerAnimInstanceBase::UpdateMovementVelocity()
{
	// Cache the pawn's current movement values.
	if (IsValid(OwningCharacter))
	{
		const FVector PawnVelocity = OwningCharacter->GetVelocity();
		SignedSpeed = OwningCharacter->GetVelocity().Length();
		
		const FVector UnrotatedVelocity = OwningCharacter->GetActorRotation().UnrotateVector(PawnVelocity);
		ForwardBackwardSpeed = UnrotatedVelocity.X;
		RightLeftSpeed = UnrotatedVelocity.Y;
		UpDownSpeed = UnrotatedVelocity.Z;
	}
}
