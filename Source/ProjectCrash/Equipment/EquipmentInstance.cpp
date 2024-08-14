// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentInstance.h"

#include "EquipmentActor.h"
#include "EquipmentSkin.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "Characters/CrashCharacter.h"
#include "GameFramework/CrashLogging.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"

UEquipmentInstance::UEquipmentInstance() :
	Instigator(nullptr),
	EquipmentDefinition(nullptr),
	EquipmentSkin(nullptr)
{
}

void UEquipmentInstance::InitializeEquipment(UEquipmentDefinition* InEquipmentDefinition, UEquipmentSkin* InEquipmentSkin)
{
	check(GetPawn()->HasAuthority());
	check(InEquipmentDefinition);
	check(InEquipmentSkin);

	// Initialize data.
	EquipmentDefinition = InEquipmentDefinition;
	EquipmentSkin = InEquipmentSkin;

	// Spawn equipment actors.
	SpawnEquipmentActors(EquipmentSkin->FirstPersonActorsToSpawn, EEquipmentPerspective::FirstPerson);
	SpawnEquipmentActors(EquipmentSkin->ThirdPersonActorsToSpawn, EEquipmentPerspective::ThirdPerson);

	// Grant ability sets.
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetPawn()))
	{
		for (const TObjectPtr<const UCrashAbilitySet>& AbilitySet : EquipmentDefinition->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(CrashASC, &GrantedAbilitySetHandles, this);
		}
	}
	// Warn if this equipment is supposed to grant ability sets but can't.
	else if (EquipmentDefinition->AbilitySetsToGrant.Num())
	{
		UE_LOG(LogEquipment, Warning, TEXT("Equipped equipment [%s] on actor [%s] but failed to grant the equipment's ability sets: could not find an ASC for the equipping actor."), *GetNameSafe(EquipmentDefinition), *GetNameSafe(GetPawn()));
	}
}

void UEquipmentInstance::UninitializeEquipment()
{
	check(GetPawn()->HasAuthority());

	// Destroy equipment actors.
	DestroyEquipmentActors();

	// Remove granted ability sets.
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetPawn()))
	{
		GrantedAbilitySetHandles.RemoveFromAbilitySystem(CrashASC);
	}
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
	// Reveal the equipment actors once the equipment has been fully replicated.
	for (AEquipmentActor* EquipmentActor : SpawnedActors)
	{
		if (IsValid(EquipmentActor))
		{
			if (USceneComponent* Root = EquipmentActor->GetRootComponent())
			{
				Root->SetHiddenInGame(false);
			}
		}
	}

	// Update the equipping pawn's meshes to use the new equipment skin's animations, and play any "equip" animations.
	if (EquipmentSkin->FirstPersonAnimInstance)
	{
		if (USkeletalMeshComponent* FirstPersonMesh = GetFirstPersonMeshFromPawn())
		{
			// Update animation blueprint.
			FirstPersonMesh->SetAnimInstanceClass(EquipmentSkin->FirstPersonAnimInstance);

			// Play "equip" animation.
			if (EquipmentSkin->FirstPersonEquipAnim)
			{
				FirstPersonMesh->GetAnimInstance()->Montage_Play(EquipmentSkin->FirstPersonEquipAnim);
			}
		}
	}
	if (EquipmentSkin->ThirdPersonAnimInstance)
	{
		if (USkeletalMeshComponent* ThirdPersonMesh = GetThirdPersonMeshFromPawn())
		{
			// Update animation blueprint.
			ThirdPersonMesh->SetAnimInstanceClass(EquipmentSkin->ThirdPersonAnimInstance);

			// Play "equip" animation.
			if (EquipmentSkin->ThirdPersonEquipAnim)
			{
				ThirdPersonMesh->GetAnimInstance()->Montage_Play(EquipmentSkin->ThirdPersonEquipAnim);
			}
		}
	}

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
			AEquipmentActor* NewActor = GetWorld()->SpawnActorDeferred<AEquipmentActor>(ActorInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
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

USkeletalMeshComponent* UEquipmentInstance::GetFirstPersonMeshFromPawn() const
{
	/* We can only use a first-person mesh if this instance is equipped on a pawn of type CrashCharacter, which has a
	 * dedicated first-person mesh. */
	if (ACrashCharacter* CrashChar = Cast<ACrashCharacter>(GetPawn()))
	{
		return CrashChar->GetFirstPersonMesh();
	}

	return nullptr;
}

USkeletalMeshComponent* UEquipmentInstance::GetThirdPersonMeshFromPawn() const
{
	APawn* Pawn = GetPawn();

	// Try to retrieve the CrashCharacter's dedicated third-person mesh.
	if (ACrashCharacter* CrashChar = Cast<ACrashCharacter>(Pawn))
	{
		return CrashChar->GetThirdPersonMesh();
	}
	// Try to retrieve the Character's generic mesh.
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		return Char->GetMesh();
	}
	// Try to retrieve any skeletal mesh on the pawn.
	if (USkeletalMeshComponent* SkelMesh = Pawn->FindComponentByClass<USkeletalMeshComponent>())
	{
		return SkelMesh;
	}

	return nullptr;
}

void UEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, EquipmentDefinition);
	DOREPLIFETIME(ThisClass, EquipmentSkin);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}
