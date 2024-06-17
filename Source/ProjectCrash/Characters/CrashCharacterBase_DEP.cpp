// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacterBase_DEP.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CrashLogging.h"


#define FIRST_PERSON_TAG CrashGameplayTags::TAG_State_Perspective_FirstPerson
#define THIRD_PERSON_TAG CrashGameplayTags::TAG_State_Perspective_ThirdPerson

ACrashCharacterBase_DEP::ACrashCharacterBase_DEP(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(CharacterMovementComponentName, UCrashCharacterMovementComponent::StaticClass()))
{
}

void ACrashCharacterBase_DEP::SetPerspective(FGameplayTag NewPerspective)
{
	// Make sure a perspective tag was given.
	check(NewPerspective.GetGameplayTagParents().HasTagExact(CrashGameplayTags::TAG_State_Perspective));

	// Do nothing if we are already in the correct perspective.
	if (NewPerspective == CharacterPerspective)
	{
		return;
	}

	// If we try to enter first-person, check if we can.
	if (NewPerspective == FIRST_PERSON_TAG && !CanEnterFirstPerson())
	{
		/* If we can't enter first-person, return to third-person. This should only be necessary if we ever add more
		 * than two character perspectives. */
		SetPerspective(THIRD_PERSON_TAG);
		return;
	}

	// If someone is viewing this character, we'll update the camera they're using. Otherwise, we don't care.
	APlayerController* PC = GetLocalViewingPlayerController();

	if (NewPerspective == FIRST_PERSON_TAG)
	{
		// A listen server will return a nullptr from GetLocalViewingPlayerController when it controls this pawn.
		if (!IsValid(PC))
		{
			PC = GetController<APlayerController>();
		}

		/* We need a local viewing controller if we want to view in first-person. This is already checked by
		 * CanEnterFirstPerson. */
		check(PC);

		GetThirdPersonMesh()->SetVisibility(false, true);
		GetFirstPersonMesh()->SetVisibility(true, true);

		GetThirdPersonCameraComponent()->Deactivate();
		GetFirstPersonCameraComponent()->Activate();

		CharacterPerspective = FIRST_PERSON_TAG;
		PerspectiveChangedDelegate.Broadcast(CharacterPerspective);
	}
	else if (NewPerspective == THIRD_PERSON_TAG)
	{
		/* This code could be written more efficiently than as-is, but this method makes it easier to add new
		 * perspectives in the future, if we ever want to. */
		GetFirstPersonMesh()->SetVisibility(false, true);
		GetThirdPersonMesh()->SetVisibility(true, true);

		GetFirstPersonCameraComponent()->Deactivate();
		GetThirdPersonCameraComponent()->Activate();

		CharacterPerspective = THIRD_PERSON_TAG;
		PerspectiveChangedDelegate.Broadcast(CharacterPerspective);
	}

	// Update the viewer's camera by refreshing their view target.
	if (PC)
	{
		PC->SetViewTarget(this);
	}
}

void ACrashCharacterBase_DEP::InitializePerspective()
{
	// Start local players in first-person. Everyone else (including spectators) starts in third-person.
	const bool bStartFPP = IsLocallyControlled();
	SetPerspective(bStartFPP ? FIRST_PERSON_TAG : THIRD_PERSON_TAG);
}

bool ACrashCharacterBase_DEP::CanEnterFirstPerson() const
{
	// Only enter first-person if we have a valid camera, mesh, and someone is actually viewing this character.
	return GetFirstPersonCameraComponent() && GetFirstPersonMesh() && (IsLocallyControlled() || IsLocallyViewed());
}

void ACrashCharacterBase_DEP::ListenForPerspectiveStates()
{
	// If this character has an ASC, it can listen for perspective events.
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		ASC->RegisterGameplayTagEvent(CrashGameplayTags::TAG_State_Perspective_FirstPerson, EGameplayTagEventType::AnyCountChange).AddUObject(this, &ACrashCharacterBase_DEP::OnPerspectiveStateChanged);
		ASC->RegisterGameplayTagEvent(CrashGameplayTags::TAG_State_Perspective_ThirdPerson, EGameplayTagEventType::AnyCountChange).AddUObject(this, &ACrashCharacterBase_DEP::OnPerspectiveStateChanged);
	}
	else
	{
		ABILITY_LOG(Error, TEXT("ACrashCharacterBase: Failed to start listening for perspective states on actor %s. ListenForPerspectiveStates was likely called before the ASC was initialized."), *GetName());
	}
}

void ACrashCharacterBase_DEP::OnPerspectiveStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	// Calculate the change in tags. Should only ever be +1 or -1.
	int32 CountChange = NewCount;
	if (PerspectiveTagsStacks.Contains(Tag))
	{
		CountChange = NewCount - PerspectiveTagsStacks[Tag];
	}

	// If this is the first time we're receiving a perspective state, switch to the requested perspective.
	if (TotalPerspectiveTags == 0)
	{
		/* Cache our current perspective before we change. We will return to this perspective when all state tags have
		 * been removed. */
		BackingCharacterPerspective = CharacterPerspective;

		// Switch to the new perspective.
		SetPerspective(Tag);
	}

	/* If all counts of a certain perspective tag have been removed, switch to another active perspective, or fall back
	 * to the backing perspective. */
	if (NewCount == 0)
	{
		PerspectiveTagsStacks.Remove(Tag);
	
		// Search for any remaining perspective overrides and switch to the first one.
		bool bTagsRemaining = false;
		for (const auto& TagCountPair : PerspectiveTagsStacks)
		{
			if (TagCountPair.Value > 0)
			{
				bTagsRemaining = true;
				SetPerspective(TagCountPair.Key);
				break;
			}
		}

		// If there are no remaining perspective overrides, return to the backing perspective.
		if (!bTagsRemaining)
		{
			SetPerspective(BackingCharacterPerspective);
		}
	}

	// Update internal tracking.
	PerspectiveTagsStacks.FindOrAdd(Tag) = NewCount;
	TotalPerspectiveTags += CountChange;
}

void ACrashCharacterBase_DEP::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	JumpedDelegate.Broadcast();
}
