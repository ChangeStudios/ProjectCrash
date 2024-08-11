// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentInstance.h"

#include "EquipmentActor.h"
#include "EquipmentSkin.h"
#include "Characters/CrashCharacter.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

UEquipmentInstance::UEquipmentInstance()
{
}

#if UE_WITH_IRIS
void UEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object.
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UEquipmentInstance::GetPawn() const
{
	// Outer object should always be the pawn on which this equipment is equipped.
	return Cast<APawn>(GetOuter());
}

APawn* UEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* OutPawn = nullptr;

	if (GetOuter()->IsA(PawnType))
	{
		OutPawn = Cast<APawn>(GetOuter());
	}

	return OutPawn;
}

void UEquipmentInstance::OnEquipped()
{
	K2_OnEquipped();
}

void UEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void UEquipmentInstance::SpawnEquipmentActors(const TArray<FEquipmentSkinActorInfo>& ActorsToSpawn, EEquipmentPerspective Perspective)
{
	if (APawn* OwningPawn = GetPawn())
	{
		// Try to identify the optimal attach targets depending on the desired perspective and the owning pawn's class.
		USceneComponent* AttachTarget = nullptr;

		// Try to use the desired perspective's dedicated mesh.
		if (ACrashCharacter* CrashChar = Cast<ACrashCharacter>(OwningPawn))
		{
			if (Perspective == EEquipmentPerspective::FirstPerson)
			{
				AttachTarget = CrashChar->GetFirstPersonMesh();
			}
			else
			{
				AttachTarget = CrashChar->GetThirdPersonMesh();
			}
		}
		/* If the equipping pawn does not have a dedicated first-person mesh, don't spawn any first-person equipment
		 * actors. */
		else if (Perspective == EEquipmentPerspective::FirstPerson)
		{
			return;
		}
		// If the pawn is a character, try to use its generic mesh for third-person equipment actors.
		else if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}
		// Fall back to the pawn's root component.
		else
		{
			AttachTarget = OwningPawn->GetRootComponent();
		}

		// Spawn each requested actor.
		for (const FEquipmentSkinActorInfo& ActorInfo : ActorsToSpawn)
		{
			// Spawn the new equipment actor. We use deferred spawning just to initialize the actor's owner.
			AEquipmentActor* NewActor = GetWorld()->SpawnActorDeferred<AEquipmentActor>(ActorInfo.ActorToSpawn, FTransform::Identity, OwningPawn, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			NewActor->FinishSpawning(FTransform::Identity, true);
			NewActor->SetActorRelativeTransform(ActorInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, ActorInfo.AttachSocket);
			NewActor->InitEquipmentPerspective(Perspective);

			SpawnedActors.Add(NewActor);
		}
	}
}

void UEquipmentInstance::DestroyEquipmentActors()
{
	for (AEquipmentActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
}

UWorld* UEquipmentInstance::GetWorld() const
{
	// Retrieve the world through this equipment's outer actor.
	if (AActor* OuterActor = Cast<AActor>(GetOuter()))
	{
		return OuterActor->GetWorld();
	}
	else
	{
		return nullptr;
	}
}

void UEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}