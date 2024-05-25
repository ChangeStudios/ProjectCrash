// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentPieceActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Characters/CrashCharacterBase.h"
#include "GameFramework/CrashLogging.h"
#include "Kismet/KismetMaterialLibrary.h"

AEquipmentPieceActor::AEquipmentPieceActor() :
	EquipmentPiece(nullptr),
	OwningEquipmentComponent(nullptr),
	Perspective(THIRD_PERSON)
{
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("MeshComp"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionProfileName("NoCollision");
}

void AEquipmentPieceActor::InitEquipmentPieceActor(const FEquipmentPiece* InEquipmentPiece, UEquipmentComponent* InOwningEquipmentComponent, FGameplayTag InEquipmentPerspective)
{
    // Validate given properties.
    check(InEquipmentPiece);
    check(InEquipmentPerspective.GetGameplayTagParents().HasTagExact(CrashGameplayTags::TAG_State_Perspective));

    // Cache given properties.
    EquipmentPiece = InEquipmentPiece;
    OwningEquipmentComponent = InOwningEquipmentComponent;
    SetOwner(OwningEquipmentComponent->GetOwner());
    Perspective = InEquipmentPerspective;

	// If this equipment actor's owner can change perspectives, listen for those changes.
	ACrashCharacterBase* CrashChar = Cast<ACrashCharacterBase>(GetOwner());
    if (CrashChar)
    {
	    CrashChar->PerspectiveChangedDelegate.AddDynamic(this, &AEquipmentPieceActor::OnPerspectiveChanged);
    }

	// Update this equipment piece's mesh and visibility.
	MeshComponent->SetSkeletalMesh(EquipmentPiece->Mesh);
	OnPerspectiveChanged(CrashChar ? CrashChar->GetCurrentPerspective() : THIRD_PERSON);
	MeshComponent->CastShadow = Perspective == THIRD_PERSON; // Only third-person equipment casts shadows.

	// Enable/disable first-person depth rendering.
	for (int MatIndex = 0; MatIndex < MeshComponent->GetNumMaterials(); MatIndex++)
	{
		UMaterialInstanceDynamic* DynamicMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, MeshComponent->GetMaterial(MatIndex));
		DynamicMat->SetScalarParameterValue("FirstPerson", Perspective == FIRST_PERSON ? 1.0f : 0.0f);
		MeshComponent->SetMaterial(MatIndex, DynamicMat);
	}
}

void AEquipmentPieceActor::MakePhysicsActor()
{
	MeshComponent->SetCollisionProfileName(FName("PhysicsActor"));
	MeshComponent->SetSimulatePhysics(true);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void AEquipmentPieceActor::OnPerspectiveChanged(FGameplayTag NewPerspective)
{
	// Equipment actors should always match the visibility of their owners.
	MeshComponent->SetVisibility(NewPerspective == Perspective);
}
