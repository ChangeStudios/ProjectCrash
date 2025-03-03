// Copyright Samuel Reitich. All rights reserved.

#include "Equipment/EquipmentMeshComponent.h"

#include "Characters/CrashCharacter.h"
#include "GameFramework/CrashLogging.h"

void UEquipmentMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	// Validation.
	if (ACrashCharacter* OwningChar = Cast<ACrashCharacter>(GetOwner()))
	{
		if (GetAttachParent() != OwningChar->GetFirstPersonMesh() &&
			GetAttachParent() != OwningChar->GetThirdPersonMesh())
		{
			UE_LOG(LogCrash, Error, TEXT("Equipment Mesh Component in actor [%s] is not attached to a character mesh component. Equipment Mesh Components should be attached to the first-person or third-person character mesh."), *GetNameSafe(GetOwner()));
		}
		else
		{
			if ((Perspective == EEquipmentPerspective::FirstPerson) && (GetAttachParent() != OwningChar->GetFirstPersonMesh()))
			{
				UE_LOG(LogCrash, Error, TEXT("Equipment Mesh Component in actor [%s] is set to first-person, but is not attached to the first-person character mesh component."), *GetNameSafe(GetOwner()));
			}
			else if ((Perspective == EEquipmentPerspective::ThirdPerson) && (GetAttachParent() != OwningChar->GetThirdPersonMesh()))
			{
				UE_LOG(LogCrash, Error, TEXT("Equipment Mesh Component in actor [%s] is set to third-person, but is not attached to the third-person character mesh component."), *GetNameSafe(GetOwner()));
			}
		}
	}
	else
	{
		UE_LOG(LogCrash, Warning, TEXT("Equipment Mesh Component is used by actor [%s], which is not a Crash Character. Equipment Mesh Components are not intended to be used outside of Crash Character classes, and may not function properly."), *GetNameSafe(GetOwner()));
	}

	// Initialize this component's perspective-based visibility.
	SetVisibility(GetAttachParent() ? GetAttachParent()->IsVisible() : true, true);

	// Enable first-person rendering if necessary.
	if (Perspective == EEquipmentPerspective::FirstPerson)
	{
		// Prevent clipping.
		SetScalarParameterValueOnMaterials(FName("FirstPerson"), 1.0f);
		// Hide shadows.
		SetCastShadow(0.0f);
		// Prevent culling when this component would be clipping through walls.
		bUseAttachParentBound = true;
	}
}
