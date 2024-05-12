// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EquipmentComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/CrashLogging.h"
#include "Kismet/KismetMaterialLibrary.h"


AEquipmentActor::AEquipmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	SetRootComponent(Root);
}

void AEquipmentActor::InitEquipmentActor(const UEquipmentComponent* InOwningEquipmentComponent, const UEquipmentPieceDefinition* InEquipmentPieceDefinition, ECharacterPerspective InEquipmentPerspective)
{
	check(InEquipmentPieceDefinition);

	// Cache this equipment actor's properties.
	OwningEquipmentComponent = InOwningEquipmentComponent;
	SetOwner(InOwningEquipmentComponent->GetOwner());
	SourceEquipmentPiece = InEquipmentPieceDefinition;
	EquipmentPerspective = InEquipmentPerspective;

	// Spawn the equipment actor's mesh.
	SpawnMeshComponent(InEquipmentPieceDefinition->Mesh, EquipmentPerspective);
}

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
	// If the event is an equipment effect, trigger the effect, if it is defined by this equipment piece.
	if (SourceEquipmentPiece->EffectMap.Contains(EventTag))
	{
		const FEquipmentEffect* Effect = SourceEquipmentPiece->EffectMap.Find(EventTag);

		// Throw out the effect if this actor is in the wrong perspective.
		if (Effect->EffectPerspective == ECharacterPerspective::FirstPerson && EquipmentPerspective == ECharacterPerspective::ThirdPerson ||
			Effect->EffectPerspective == ECharacterPerspective::ThirdPerson && EquipmentPerspective == ECharacterPerspective::FirstPerson)
		{
			return;
		}

		// If the effect has a cue, try to trigger the cue.
		if (UCrashAbilitySystemComponent* CrashASC = Effect->GameplayCue.IsValid() ? Cast<UCrashAbilitySystemComponent>(GetASCFromEquipmentComponent(OwningEquipmentComponent)) : nullptr)
		{
			// Throw out the effect if the actor's local role is not correct.
			const bool bLocallyControlled = Cast<APawn>(CrashASC->GetAvatarActor())->IsLocallyControlled();
			if (Effect->EffectPerspective == ECharacterPerspective::FirstPerson && !bLocallyControlled ||
				Effect->EffectPerspective == ECharacterPerspective::ThirdPerson && bLocallyControlled ||
				EquipmentPerspective == ECharacterPerspective::FirstPerson && !bLocallyControlled ||
				EquipmentPerspective == ECharacterPerspective::ThirdPerson && bLocallyControlled)
			{
				return;
			}

			// Trigger the cue if all checks pass.
			FGameplayCueParameters CueParams = FGameplayCueParameters();
			CueParams.EffectContext = CrashASC->MakeEffectContext();
			CueParams.Instigator = CrashASC->GetOwner();
			CueParams.EffectCauser = CrashASC->GetAvatarActor();
			CueParams.TargetAttachComponent = Effect->bAttachToSocket ? Mesh : nullptr;
			CueParams.Location = Effect->bAttachToSocket ? Effect->Offset : Mesh->GetSocketLocation(Effect->Socket) + Effect->Offset;

			if (Effect->bAddCue)
			{
				CrashASC->AddGameplayCueLocal(Effect->GameplayCue, CueParams);
				ActiveEffectCues.AddUnique(Effect->GameplayCue);
			}
			else
			{
				CrashASC->ExecuteGameplayCueLocal(Effect->GameplayCue, CueParams);
			}
		}

		// If the effect triggers an animation, play the animation.
		if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(Mesh))
		{
			SkeletalMesh->PlayAnimation(Effect->MeshAnimation, false);
		}
	}
}

void AEquipmentActor::EndEquipmentEvent(FGameplayTag EventTag)
{
	if (const FEquipmentEffect* Effect = SourceEquipmentPiece->EffectMap.Find(EventTag))
	{
		const FGameplayTag EventCue = Effect->GameplayCue;

		// Remove the event's active cues, if it has any.
		if (Effect->bAddCue && ActiveEffectCues.Contains(EventCue))
		{
			if (UCrashAbilitySystemComponent* CrashASC = Cast<UCrashAbilitySystemComponent>(GetASCFromEquipmentComponent(OwningEquipmentComponent)))
			{
				FGameplayCueParameters CueParams = FGameplayCueParameters();
				CueParams.EffectContext = CrashASC->MakeEffectContext();
				CueParams.Instigator = CrashASC->GetOwner();
				CueParams.EffectCauser = CrashASC->GetAvatarActor();

				CrashASC->RemoveGameplayCueLocal(EventCue, CueParams);
				ActiveEffectCues.Remove(EventCue);
			}
		}
	}
}

void AEquipmentActor::SpawnMeshComponent(UStreamableRenderAsset* InMesh, ECharacterPerspective InEquipmentPerspective)
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

	// Initialize the new mesh component's properties.
	Mesh->SetOnlyOwnerSee(InEquipmentPerspective == ECharacterPerspective::FirstPerson);
	Mesh->SetOwnerNoSee(InEquipmentPerspective == ECharacterPerspective::ThirdPerson);
	Mesh->SetCollisionProfileName("NoCollision");

	// Enable/disable first-person depth rendering.
	for (int MatIndex = 0; MatIndex < Mesh->GetNumMaterials(); MatIndex++)
	{
		UMaterialInstanceDynamic* DynamicMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, Mesh->GetMaterial(MatIndex));
		Mesh->SetMaterial(MatIndex, DynamicMat);
		DynamicMat->SetScalarParameterValue("FirstPerson", InEquipmentPerspective == ECharacterPerspective::FirstPerson ? 1.0f : 0.0f);
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
