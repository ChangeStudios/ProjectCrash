// Copyright Samuel Reitich. All rights reserved.

#include "Equipment/EquipmentMeshComponent.h"

#include "Characters/CrashCharacter.h"
#include "GameFramework/CrashLogging.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif

void UEquipmentMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	ACrashCharacter* OwningChar = Cast<ACrashCharacter>(GetOwner());
	check(OwningChar);

	// Cache which perspective this mesh is used for.
	if (GetAttachParent() == OwningChar->GetFirstPersonMesh())
	{
		Perspective = EEquipmentPerspective::FirstPerson;
	}
	else if (GetAttachParent() == OwningChar->GetThirdPersonMesh())
	{
		Perspective = EEquipmentPerspective::ThirdPerson;
	}
	else
	{
		EQUIPMENT_LOG(Fatal, TEXT("Equipment Mesh Component on actor [%s] is not attached to a character mesh component. Perspective cannot be initialized."), *GetNameSafe(GetOwner()));
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

void UEquipmentMeshComponent::OnRegister()
{
	Super::OnRegister();

	if (ACrashCharacter* OwningChar = Cast<ACrashCharacter>(GetOwner()))
	{
		// Make sure this component is attached to a character mesh.
		if (GetAttachParent() != OwningChar->GetFirstPersonMesh() &&
			GetAttachParent() != OwningChar->GetThirdPersonMesh())
		{
			EQUIPMENT_LOG(Error, TEXT("Equipment Mesh Component in actor [%s] is not attached to a character mesh component. Equipment Mesh Components should be attached to the first-person or third-person character mesh."), *GetNameSafe(GetOwner()));
#if WITH_EDITOR
			if (GIsEditor)
			{
				static const FText Message = NSLOCTEXT("EquipmentMeshComponent", "NotAttachedToCharMeshError", "is not attached to a character mesh component. Equipment Mesh Components should be attached to the first-person or third-person character mesh.");
				static const FName EquipmentMeshComponentMessageLogName = TEXT("EquipmentMeshComponent");

				FMessageLog(EquipmentMeshComponentMessageLogName).Error()
					->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
					->AddToken(FTextToken::Create(Message));

				FMessageLog(EquipmentMeshComponentMessageLogName).Open();
			}
#endif
		}
	}
	else
	{
		// Make sure this component is only added to CrashCharacter actors.
		EQUIPMENT_LOG(Warning, TEXT("Equipment Mesh Component was added to [%s], which is not a Crash Character. Equipment Mesh Components can only be used in Crash Character classes."), *GetNameSafe(GetOwner()));
#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("EquipmentMeshComponent", "NotOnCrashCharError", "was added to a class that is not a Crash Character. Equipment Mesh Components can only be used in Crash Character classes.");
			static const FName EquipmentMeshComponentMessageLogName = TEXT("EquipmentMeshComponent");

			FMessageLog(EquipmentMeshComponentMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(EquipmentMeshComponentMessageLogName).Open();
		}
#endif
	}
}
