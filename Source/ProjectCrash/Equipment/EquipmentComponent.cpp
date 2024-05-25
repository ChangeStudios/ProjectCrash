// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Animation/ChallengerAnimInstanceBase.h"
#include "EquipmentPieceActor.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "Characters/CrashCharacterBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerStates/CrashPlayerState.h"

UEquipmentComponent::UEquipmentComponent() :
	EquippedSetHandle(FEquipmentSetHandle()),
	TemporarilyEquippedSetHandle(FEquipmentSetHandle())
{
	// Equipment is server-authoritative and replicated.
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that this component is owned by a character class. It cannot be used by any other classes.
	const ACharacter* Char = GetOwner<ACharacter>();
	ensureAlwaysMsgf((Char != nullptr), TEXT("EquipmentComponent was registered on [%s]. This component can only be used on classes of the ACharacter type."), *GetNameSafe(GetOwner()));

	// Ensure that this component's owner only has one EquipmentComponent.
	TArray<UActorComponent*> EquipmentComponents;
	Char->GetComponents(UEquipmentComponent::StaticClass(), EquipmentComponents);
	ensureAlwaysMsgf((EquipmentComponents.Num() == 1), TEXT("%i instances of EquipmentComponent were found on %s. Only one EquipmentComponent may exist on any actor."), EquipmentComponents.Num(), *GetNameSafe(GetOwner()));
}

void UEquipmentComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (HasAuthority())
	{
		// Unequip the temporary set, if one exists.
		UnequipTemporarySet();
	}

	// Forcefully unequip any backing set.
	UnequipSet_Internal(false, false);

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UEquipmentComponent::EquipSet(UEquipmentSetDefinition* SetToEquip)
{
	check(HasAuthority());
	check(SetToEquip);

	/* Update the currently equipped set. The previously equipped set will be unequipped by the OnRep, and the new set
	 * will be equipped. */
	UEquipmentSetDefinition* PreviouslyEquippedSet = EquippedSet;
	EquippedSet = SetToEquip;

	// Manually call the OnRep on the server.
	OnRep_EquippedSet(PreviouslyEquippedSet);
}

void UEquipmentComponent::TemporarilyEquipSet(UEquipmentSetDefinition* SetToTemporarilyEquip)
{
	check(HasAuthority());
	check(SetToTemporarilyEquip);

	/* Update the currently equipped set. If there was a temporarily equipped set, it will be unequipped by the OnRep;
	 * otherwise, EquippedSet will be unequipped instead, before the new temporary set is equipped. */
	UEquipmentSetDefinition* PreviousTemporarilyEquippedSet = TemporarilyEquippedSet;
	TemporarilyEquippedSet = SetToTemporarilyEquip;

	// Manually call the OnRep on the server.
	OnRep_TemporarilyEquippedSet(PreviousTemporarilyEquippedSet);
}

bool UEquipmentComponent::UnequipTemporarySet()
{
	check(HasAuthority());

	// Unequip the current temporarily equipped set, if there is one.
	if (UEquipmentSetDefinition* PreviousTemporarilyEquippedSet = TemporarilyEquippedSet)
	{
		TemporarilyEquippedSet = nullptr;
		OnRep_TemporarilyEquippedSet(PreviousTemporarilyEquippedSet);
		return true;
	}

	// If there isn't a temporarily equipped set, do nothing.
	return false;
}

void UEquipmentComponent::DetachEquipment()
{
	// Detach all temporary equipment actors.
	for (AEquipmentPieceActor* EquipmentActor : TemporarilyEquippedSetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->DetachEquipmentActor();
	}

	// Detach all equipment actors.
	for (AEquipmentPieceActor* EquipmentActor : EquippedSetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->DetachEquipmentActor();
	}
}

void UEquipmentComponent::EquipSet_Internal(UEquipmentSetDefinition* SetToEquip, bool bEquipAsTemporarySet, bool bWasTemporarilyUnequipped)
{
	check(SetToEquip);

	if (!SetToEquip->SetID.IsValid())
	{
		EQUIPMENT_LOG(Fatal, TEXT("Attempted to equip set without a valid set ID: %s"), *SetToEquip->GetName());
		return;
	}

	// Cache for convenience.
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();

	if (!Owner || !World)
	{
		return;
	}

	// Try to get the equipping actor as characters for character-specific equipment logic (e.g. updating animations).
	ACharacter* EquippingChar = Cast<ACharacter>(Owner);
	ACrashCharacterBase* EquippingCrashChar = Cast<ACrashCharacterBase>(Owner);

	// Attempt to retrieve the owning character's skin in order to retrieve the skin data for the equipping set.
	UEquipmentSetSkinData* EquippingSetSkinData = SetToEquip->DefaultSkinData;

	if (const APawn* OwnerAsPawn = Cast<APawn>(Owner))
	{
		ACrashPlayerState* CrashPS = OwnerAsPawn->GetPlayerState<ACrashPlayerState>();
		UChallengerSkinData* ChallengerSkin = CrashPS ? CrashPS->GetCurrentSkin() : nullptr;
		if (ChallengerSkin && ChallengerSkin->EquipmentSetSkins.Contains(SetToEquip->SetID))
		{
			EquippingSetSkinData = *ChallengerSkin->EquipmentSetSkins.Find(SetToEquip->SetID);
		}
	}

	/* We should always have skin data for an equipment set, either from a character skin or from the set's default
	 * data. */
	if (!ensure(EquippingSetSkinData))
	{
		return;
	}

	// Determine which equipment handle to store the equipped set into.
	FEquipmentSetHandle& TargetHandle = bEquipAsTemporarySet ? TemporarilyEquippedSetHandle : EquippedSetHandle;



	// Cache the set, if it hasn't been already. If the set is being re-equipped, it should already be cached.
	if (!bWasTemporarilyUnequipped)
	{
		TargetHandle.EquipmentSetDefinition = SetToEquip;
	}



	// Grant or re-enable the new equipment's ability set on the server.
	if (HasAuthority() && SetToEquip->GrantedAbilitySet)
	{
		if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(Owner))
		{
			/* If this set was only unequipped temporarily, we still have our abilities granted and cached; we just
			 * need to re-enable them. */
			if (bWasTemporarilyUnequipped)
			{
				SetToEquip->GrantedAbilitySet->GiveToAbilitySystem(CrashASC, nullptr, this, true);
			}
			// If we're equipping this set from scratch, grant and cache its ability set.
			else
			{
				SetToEquip->GrantedAbilitySet->GiveToAbilitySystem(CrashASC, &TargetHandle.GrantedAbilitySetHandle, this);
				TargetHandle.GrantedASC = CrashASC;
			}
		}
	}



	/* Visually equip the set (spawn its pieces and switch animations) if (A) we're equipping an overriding temporary
	 * set or (B) we're equipping a primary equipment set and there is no overriding temporary set. */
	if (bEquipAsTemporarySet || (!bEquipAsTemporarySet && TemporarilyEquippedSet == nullptr))
	{
		/* Before we spawn our equipment set pieces, choose where we'll attach our third-person pieces. By default, use
		 * the equipping actor's root component. */
		USceneComponent* AttachComponent = Owner->GetRootComponent();

		// Try to retrieve the defined third-person mesh.
		if (EquippingCrashChar && EquippingCrashChar->GetThirdPersonMesh())
		{
			AttachComponent = EquippingCrashChar->GetThirdPersonMesh();
		}
		// Try to retrieve any character mesh.
		else if (EquippingChar && EquippingChar->GetMesh())
		{
			AttachComponent = EquippingChar->GetMesh();
		}
		// Try to retrieve ANY mesh.
		else if (UMeshComponent* MeshComp = Owner->FindComponentByClass<UMeshComponent>())
		{
			AttachComponent = MeshComp;
		}


		// Spawn the new equipment set's pieces.
		for (FEquipmentPiece& EquipmentPiece : EquippingSetSkinData->Pieces)
		{
			// Spawn the equipment piece's first-person actor if the equipping character has one.
			if (USkeletalMeshComponent* Mesh_FPP = EquippingCrashChar ? EquippingCrashChar->GetFirstPersonMesh() : nullptr)
			{
				if (AEquipmentPieceActor* NewPieceActor = World->SpawnActor<AEquipmentPieceActor>())
				{
					NewPieceActor->InitEquipmentPieceActor(&EquipmentPiece, this, CrashGameplayTags::TAG_State_Perspective_FirstPerson);
					NewPieceActor->AttachToComponent(Mesh_FPP, FAttachmentTransformRules::SnapToTargetIncludingScale, EquipmentPiece.AttachSocket);
					NewPieceActor->SetActorRelativeTransform(EquipmentPiece.Offset_FPP);

					TargetHandle.SpawnedEquipmentActors.Add(NewPieceActor);
				}
			}


			// Always spawn the third-person actor.
			if (AEquipmentPieceActor* NewPieceActor = World->SpawnActor<AEquipmentPieceActor>())
			{
				NewPieceActor->InitEquipmentPieceActor(&EquipmentPiece, this, CrashGameplayTags::TAG_State_Perspective_ThirdPerson);
				NewPieceActor->AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, EquipmentPiece.AttachSocket);
				NewPieceActor->SetActorRelativeTransform(EquipmentPiece.Offset_TPP);

				TargetHandle.SpawnedEquipmentActors.Add(NewPieceActor);
			}
		}



		// Update the equipping character's animation to use the new set's animations.
		if (SetToEquip->AnimationData)
		{
			// Optional first-person mesh.
			if (UChallengerAnimInstanceBase* FPPAnimInstance = (EquippingCrashChar && EquippingCrashChar->GetFirstPersonMesh()) ?
																	Cast<UChallengerAnimInstanceBase>(EquippingCrashChar->GetFirstPersonMesh()->GetAnimInstance()) :
																	nullptr)
			{
				FPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData, EquippingSetSkinData);

				// Play the first-person "equip" montage.
				if (EquippingSetSkinData->EquipAnim_FPP)
				{
					FPPAnimInstance->Montage_Play(EquippingSetSkinData->EquipAnim_FPP);
				}
			}

			// Third-person mesh. AttachComponent has already done the work of searching for a mesh component for us.
			if (const USkeletalMeshComponent* MeshComp = (Cast<USkeletalMeshComponent>(AttachComponent)))
			{
				if (UChallengerAnimInstanceBase* TPPAnimInstance = Cast<UChallengerAnimInstanceBase>(MeshComp->GetAnimInstance()))
				{
					TPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData, EquippingSetSkinData);

					// Play the third-person "equip" montage.
					if (EquippingSetSkinData->EquipAnim_TPP)
					{
						TPPAnimInstance->Montage_Play(EquippingSetSkinData->EquipAnim_TPP);
					}
				}
			}
		}
	}
}

void UEquipmentComponent::UnequipSet_Internal(bool bUnequipTemporarySet, bool bUnequipTemporarily)
{
	if (!GetOwner())
	{
		return;
	}

	// Determine which equipment handle to use to unequip the set.
	FEquipmentSetHandle& TargetHandle = bUnequipTemporarySet ? TemporarilyEquippedSetHandle : EquippedSetHandle;



	// Remove or disable the unequipped set's ability set on the server.
	if (HasAuthority() && TargetHandle.GrantedASC)
	{
		// If we're only temporarily unequipping this set, we disable the granted ability set instead of removing it.
		if (bUnequipTemporarily)
		{
			TargetHandle.GrantedAbilitySetHandle.RemoveFromAbilitySystem(TargetHandle.GrantedASC, true);
		}
		// If we're completely unequipping this set, remove its ability set and null its granted ASC.
		else
		{
			TargetHandle.GrantedAbilitySetHandle.RemoveFromAbilitySystem(TargetHandle.GrantedASC);
			TargetHandle.GrantedASC = nullptr;
		}
	}



	// Destroy the unequipped set's spawned actors.
	for (AEquipmentPieceActor* EquipmentActor : TargetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->Destroy();
	}

	TargetHandle.SpawnedEquipmentActors.Reset();
}

void UEquipmentComponent::OnRep_EquippedSet(UEquipmentSetDefinition* PreviouslyEquippedSet)
{
	// Unequip the previously equipped set, if there was one.
	if (PreviouslyEquippedSet)
	{
		UnequipSet_Internal(false, false);
	}

	// Equip the new set.
	if (EquippedSet)
	{
		EquipSet_Internal(EquippedSet, false, false);
	}
}

void UEquipmentComponent::OnRep_TemporarilyEquippedSet(UEquipmentSetDefinition* PreviouslyTemporarilyEquippedSet)
{
	// If a new set was just temporarily equipped, equip it.
	if (TemporarilyEquippedSet)
	{
		EquipSet_Internal(TemporarilyEquippedSet, true, false);
	}
	// If the temporarily equipped set was just unequipped, re-equip EquippedSet.
	else
	{
		EquipSet_Internal(EquippedSet, false, true);
	}

	// If a temporary set was previously equipped, unequip it.
	if (PreviouslyTemporarilyEquippedSet)
	{
		UnequipSet_Internal(true, false);
	}
	/* If a persistent equipment set was previously equipped, and a new temporary set is overriding it, temporarily
	 * unequip the persistent set. */
	else if (TemporarilyEquippedSet)
	{
		UnequipSet_Internal(false, true);
	}
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, EquippedSet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, TemporarilyEquippedSet, COND_None, REPNOTIFY_Always);
}
