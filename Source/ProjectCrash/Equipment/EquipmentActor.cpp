// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EquipmentComponent.h"


AEquipmentActor::AEquipmentActor() :
	Mesh(nullptr),
	EquipmentPerspective(ECharacterPerspective::None)
{
}

void AEquipmentActor::InitEquipmentActor(const UEquipmentComponent* InOwningEquipmentComponent, const UEquipmentPieceDefinition* InEquipmentPieceDefinition, ECharacterPerspective InEquipmentPerspective)
{
	check(InEquipmentPieceDefinition);

	// Cache this equipment actor's parameters.
	OwningEquipmentComponent = InOwningEquipmentComponent;
	SourceEquipmentPiece = InEquipmentPieceDefinition;
	EquipmentPerspective = InEquipmentPerspective;

	// Spawn the equipment actor's mesh.
	Mesh = SpawnMeshComponent(SourceEquipmentPiece->Mesh, EquipmentPerspective);
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

		// If the effect has a cue, trigger the cue.
		if (UAbilitySystemComponent* ASC = Effect->GameplayCue.IsValid() ? GetASCFromEquipmentComponent(OwningEquipmentComponent) : nullptr)
		{
			FGameplayCueParameters CueParams = FGameplayCueParameters();
			CueParams.EffectContext = ASC->MakeEffectContext();
			CueParams.Instigator = ASC->GetOwner();
			CueParams.EffectCauser = ASC->GetAvatarActor();
			CueParams.TargetAttachComponent = Effect->bAttachToSocket ? Mesh : nullptr;
			CueParams.Location = Effect->bAttachToSocket ? Effect->Offset : Mesh->GetSocketLocation(Effect->Socket) + Effect->Offset;

			if (Effect->bAddCue)
			{
				ASC->AddGameplayCue(Effect->GameplayCue, CueParams);
				ActiveEffectCues.Add(Effect->GameplayCue);
			}
			else
			{
				ASC->ExecuteGameplayCue(Effect->GameplayCue, CueParams);
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
	const FGameplayTag EventCue = SourceEquipmentPiece->EffectMap.Find(EventTag)->GameplayCue;

	// Remove the event's active cues, if it has any.
	if (ActiveEffectCues.Contains(EventCue))
	{
		if (UAbilitySystemComponent* ASC = GetASCFromEquipmentComponent(OwningEquipmentComponent))
		{
			ASC->RemoveGameplayCue(EventCue);
		}
	}
}

UMeshComponent* AEquipmentActor::SpawnMeshComponent(TObjectPtr<UStreamableRenderAsset> InMesh, ECharacterPerspective InEquipmentPerspective)
{
	// Spawn the mesh if it's a static mesh.
	if (Mesh.GetClass() == UStaticMesh::StaticClass())
	{
		if (UStaticMeshComponent* StaticMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass()))
		{
			StaticMesh->SetOnlyOwnerSee(InEquipmentPerspective == ECharacterPerspective::FirstPerson);
			StaticMesh->SetOwnerNoSee(InEquipmentPerspective == ECharacterPerspective::ThirdPerson);
	
			StaticMesh->SetStaticMesh(Cast<UStaticMesh>(InMesh));

			return StaticMesh;
		}
	}
	// Spawn the mesh if it's a skeletal mesh.
	else if (Mesh.GetClass() == USkeletalMesh::StaticClass())
	{
		if (USkeletalMeshComponent* SkeletalMesh = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass()))
		{
			SkeletalMesh->SetOnlyOwnerSee(InEquipmentPerspective == ECharacterPerspective::FirstPerson);
			SkeletalMesh->SetOwnerNoSee(InEquipmentPerspective == ECharacterPerspective::ThirdPerson);

			SkeletalMesh->SetSkeletalMesh(Cast<USkeletalMesh>(InMesh));

			return SkeletalMesh;
		}
	}

	return nullptr;
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
