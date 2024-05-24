// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Characters/CrashCharacterBase.h"
#include "EquipmentComponent.h"
#include "Kismet/KismetMaterialLibrary.h"


#define FIRST_PERSON_TAG CrashGameplayTags::TAG_State_Perspective_FirstPerson
#define THIRD_PERSON_TAG CrashGameplayTags::TAG_State_Perspective_ThirdPerson

AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(Root);
}
//
// void AEquipmentActor::InitEquipmentActor(const UEquipmentComponent* InOwningEquipmentComponent, const UEquipmentPieceDefinition* InEquipmentPieceDefinition, FGameplayTag InEquipmentPerspective)
// {
// 	check(InEquipmentPieceDefinition);
//
// 	// Make sure a perspective tag was given.
// 	check(InEquipmentPerspective.GetGameplayTagParents().HasTagExact(CrashGameplayTags::TAG_State_Perspective));
//
// 	// Cache this equipment actor's properties.
// 	OwningEquipmentComponent = InOwningEquipmentComponent;
// 	SetOwner(InOwningEquipmentComponent->GetOwner());
// 	// SourceEquipmentPiece = InEquipmentPieceDefinition;
// 	EquipmentPerspective = InEquipmentPerspective;
//
// 	/* If our owning character is an ACrashCharacterBase, we can use perspectives. Start listening for perspective
// 	 * changes if we can. */
// 	OwningCharacter = Cast<ACrashCharacterBase>(InOwningEquipmentComponent->GetOwner());
// 	if (OwningCharacter)
// 	{
// 		OwningCharacter->PerspectiveChangedDelegate.AddDynamic(this, &AEquipmentActor::OnOuterPerspectiveChanged);
// 	}
//
// 	// Spawn the equipment actor's mesh.
// 	// SpawnMeshComponent(InEquipmentPieceDefinition->Mesh);
// }

void AEquipmentActor::OnUnequip()
{
	// Remove all ongoing gameplay cues for this equipment actor.
	for (const FGameplayTag Tag : ActiveEffectCues)
	{
		EndEquipmentEvent(Tag);
	}
}

void AEquipmentActor::HandleEquipmentEvent(FGameplayTag EventTag)
{
	// // If the event is an equipment effect, trigger the effect, if it is defined by this equipment piece.
	// if (SourceEquipmentPiece->EffectMap.Contains(EventTag))
	// {
	// 	const FEquipmentEffect* Effect = SourceEquipmentPiece->EffectMap.Find(EventTag);
	//
	// 	// Throw out the effect if this actor is in the wrong perspective.
	// 	if (Effect->EffectPerspective != EquipmentPerspective)
	// 	{
	// 		return;
	// 	}
	//
	// 	// If the effect has a cue, try to trigger the cue.
	// 	if (UCrashAbilitySystemComponent* CrashASC = Effect->GameplayCue.IsValid() ? Cast<UCrashAbilitySystemComponent>(GetASCFromEquipmentComponent(OwningEquipmentComponent)) : nullptr)
	// 	{
	// 		// Trigger the cue if all checks pass.
	// 		FGameplayCueParameters CueParams = FGameplayCueParameters();
	// 		CueParams.EffectContext = CrashASC->MakeEffectContext();
	// 		CueParams.Instigator = CrashASC->GetOwner();
	// 		CueParams.EffectCauser = CrashASC->GetAvatarActor();
	// 		CueParams.TargetAttachComponent = Effect->bAttachToSocket ? Mesh : nullptr;
	// 		CueParams.Location = Effect->bAttachToSocket ? Effect->Offset : Mesh->GetSocketLocation(Effect->Socket) + Effect->Offset;
	//
	// 		if (Effect->bAddCue)
	// 		{
	// 			CrashASC->AddGameplayCueLocal(Effect->GameplayCue, CueParams);
	// 			ActiveEffectCues.AddUnique(Effect->GameplayCue);
	// 		}
	// 		else
	// 		{
	// 			CrashASC->ExecuteGameplayCueLocal(Effect->GameplayCue, CueParams);
	// 		}
	// 	}
	//
	// 	// If the effect triggers an animation, play the animation.
	// 	if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Mesh))
	// 	{
	// 		SkeletalMesh->PlayAnimation(Effect->MeshAnimation, false);
	// 	}
	// }
}

void AEquipmentActor::EndEquipmentEvent(FGameplayTag EventTag)
{
	// if (const FEquipmentEffect* Effect = SourceEquipmentPiece->EffectMap.Find(EventTag))
	// {
	// 	const FGameplayTag EventCue = Effect->GameplayCue;
	//
	// 	// Remove the event's active cues, if it has any.
	// 	if (Effect->bAddCue && ActiveEffectCues.Contains(EventCue))
	// 	{
	// 		if (UCrashAbilitySystemComponent* CrashASC = Cast<UCrashAbilitySystemComponent>(GetASCFromEquipmentComponent(OwningEquipmentComponent)))
	// 		{
	// 			FGameplayCueParameters CueParams = FGameplayCueParameters();
	// 			CueParams.EffectContext = CrashASC->MakeEffectContext();
	// 			CueParams.Instigator = CrashASC->GetOwner();
	// 			CueParams.EffectCauser = CrashASC->GetAvatarActor();
	//
	// 			CrashASC->RemoveGameplayCueLocal(EventCue, CueParams);
	// 			ActiveEffectCues.Remove(EventCue);
	// 		}
	// 	}
	// }
}

void AEquipmentActor::SpawnMeshComponent(UStreamableRenderAsset* InMesh)
{
	// Spawn the mesh if it's a static mesh.
	if (UStaticMesh* InStaticMesh = Cast<UStaticMesh>(InMesh))
	{
		if (UStaticMeshComponent* StaticMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent")))
		{
			StaticMesh->RegisterComponent();
			StaticMesh->SetStaticMesh(InStaticMesh);
			StaticMesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Mesh = StaticMesh;
		}
	}
	// Spawn the mesh if it's a skeletal mesh.
	else if (USkeletalMesh* InSkeletalMesh = Cast<USkeletalMesh>(InMesh))
	{
		if (USkeletalMeshComponent* SkeletalMesh = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), TEXT("MeshComponent")))
		{
			SkeletalMesh->RegisterComponent();
			SkeletalMesh->SetSkeletalMesh(InSkeletalMesh);
			SkeletalMesh->AttachToComponent(Root, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Mesh = SkeletalMesh;
		}
	}

	// Initialize the new mesh component's properties. Equipment without an owning character is always visible.
	Mesh->SetVisibility(OwningCharacter ? EquipmentPerspective == OwningCharacter->GetCurrentPerspective() : true);
	Mesh->SetCollisionProfileName("NoCollision");
	Mesh->CastShadow = EquipmentPerspective != FIRST_PERSON_TAG;

	// Enable/disable first-person depth rendering.
	for (int MatIndex = 0; MatIndex < Mesh->GetNumMaterials(); MatIndex++)
	{
		UMaterialInstanceDynamic* DynamicMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Mesh->GetMaterial(MatIndex));
		DynamicMat->SetScalarParameterValue("FirstPerson", EquipmentPerspective == FIRST_PERSON_TAG ? 1.0f : 0.0f);
		Mesh->SetMaterial(MatIndex, DynamicMat);
	}
}

UAbilitySystemComponent* AEquipmentActor::GetASCFromEquipmentComponent(const UEquipmentComponent* InEquipmentComponent)
{
	// Search for an ASC owned by the given equipment component's owner.
	if (const AActor* Owner = InEquipmentComponent ? InEquipmentComponent->GetOwner() : nullptr)
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	}

	return nullptr;
}

void AEquipmentActor::OnOuterPerspectiveChanged(FGameplayTag NewPerspective)
{
	// Update the mesh's visibility depending on the character's new perspective.
	Mesh->SetVisibility(EquipmentPerspective == NewPerspective);
}
