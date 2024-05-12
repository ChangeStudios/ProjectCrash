// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Animation/ChallengerAnimInstanceBase.h"
#include "Animation/AnimData/CharacterAnimData.h"
#include "EquipmentActor.h"
#include "EquipmentPieceDefinition.h"
#include "Equipment/EquipmentAnimationData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"

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

void UEquipmentComponent::SendEquipmentEffectEvent(FGameplayTag EffectEvent)
{
	// Send the event to every active equipment actor.
	for (AEquipmentActor* EquipmentActor : EquippedSetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->HandleEquipmentEvent(EffectEvent);
	}

	// Send the event to every active temporary equipment actor.
	for (AEquipmentActor* TemporaryEquipmentActor : TemporarilyEquippedSetHandle.SpawnedEquipmentActors)
	{
		TemporaryEquipmentActor->HandleEquipmentEvent(EffectEvent);
	}
}

void UEquipmentComponent::EndEquipmentEffectEvent(FGameplayTag EffectEvent)
{
	// End the event on every active equipment actor.
	for (AEquipmentActor* EquipmentActor : EquippedSetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->EndEquipmentEvent(EffectEvent);
	}

	// End the event on every active temporary equipment actor.
	for (AEquipmentActor* TemporaryEquipmentActor : TemporarilyEquippedSetHandle.SpawnedEquipmentActors)
	{
		TemporaryEquipmentActor->EndEquipmentEvent(EffectEvent);
	}
}

void UEquipmentComponent::EquipSet(UEquipmentSetDefinition* SetToEquip)
{
 	check(GetOwner()->HasAuthority());
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
	check(GetOwner()->HasAuthority());
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
	check(GetOwner()->HasAuthority());

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

void UEquipmentComponent::EquipSet_Internal(UEquipmentSetDefinition* SetToEquip, bool bEquipAsTemporarySet, bool bWasTemporarilyUnequipped)
{
	check(SetToEquip);

	if (!GetOwner() || !GetWorld())
	{
		return;
	}

	// Make sure we have a valid owning character.
	ACharacter* EquippingChar = Cast<ACharacter>(GetOwner());
	check(EquippingChar);
	ACrashCharacterBase* EquippingCrashChar = Cast<ACrashCharacterBase>(GetOwner());

	// Determine which equipment handle to store the equipped set into.
	FEquipmentSetHandle& TargetHandle = bEquipAsTemporarySet ? TemporarilyEquippedSetHandle : EquippedSetHandle;


	// Cache the set, if it hasn't been already. If the set is being re-equipped, it should already be cached.
	if (!bWasTemporarilyUnequipped)
	{
		TargetHandle.EquipmentSetDefinition = SetToEquip;
	}


	// Grant or re-enable the new equipment's ability set on the server.
	if (GetOwner()->HasAuthority() && SetToEquip->GrantedAbilitySet)
	{
		if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
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
		// Spawn the new equipment set's pieces.
		for (const UEquipmentPieceDefinition* EquipmentPiece : SetToEquip->EquipmentPieces)
		{
			// Spawn the equipment piece's first-person actor if the equipping character has one.
			if (EquippingCrashChar && EquippingCrashChar->GetFirstPersonMesh())
			{
				// Note: May need to use SpawnActorDeferred if there's any delay between spawning and initialization.
				if (AEquipmentActor* EquipmentActor_FPP = GetWorld()->SpawnActor<AEquipmentActor>())
				{
					EquipmentActor_FPP->InitEquipmentActor(this, EquipmentPiece, ECharacterPerspective::FirstPerson);
					EquipmentActor_FPP->AttachToComponent(EquippingCrashChar->GetFirstPersonMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, EquipmentPiece->AttachSocket);
					EquipmentActor_FPP->SetActorRelativeTransform(EquipmentPiece->AttachOffset_FPP);

					TargetHandle.SpawnedEquipmentActors.Add(EquipmentActor_FPP);
				}
			}

			/* Determine where to attach the equipment piece's third-person actor. If the equipping character has a
			 * valid designated third-person mesh, use it. Otherwise, fall back to using any mesh it has. */
			USceneComponent* AttachComponent = EquippingCrashChar && EquippingCrashChar->GetThirdPersonMesh() ?
													EquippingCrashChar->GetThirdPersonMesh() :
													EquippingChar->GetMesh();
			check(AttachComponent);

			// Spawn the equipment piece's third-person actor.
			if (AEquipmentActor* EquipmentActor_TPP = GetWorld()->SpawnActor<AEquipmentActor>())
			{
				EquipmentActor_TPP->InitEquipmentActor(this, EquipmentPiece, ECharacterPerspective::ThirdPerson);
				EquipmentActor_TPP->AttachToComponent(AttachComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, EquipmentPiece->AttachSocket);
				EquipmentActor_TPP->SetActorRelativeTransform(EquipmentPiece->AttachOffset_TPP);

				TargetHandle.SpawnedEquipmentActors.Add(EquipmentActor_TPP);
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
				FPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData);

				// Play the first-person "equip" montage.
				if (SetToEquip->AnimationData->Equip_FPP)
				{
					FPPAnimInstance->Montage_Play(SetToEquip->AnimationData->Equip_FPP);
				}
			}

			// Third-person mesh.
			if (UChallengerAnimInstanceBase* TPPAnimInstance = (Cast<UChallengerAnimInstanceBase>(EquippingCrashChar && EquippingCrashChar->GetThirdPersonMesh() ?
																	EquippingCrashChar->GetThirdPersonMesh()->GetAnimInstance() :
																	EquippingChar->GetMesh()->GetAnimInstance())))
			{
				TPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData);

				// Play the third-person "equip" montage.
				if (SetToEquip->AnimationData->Equip_TPP)
				{
					TPPAnimInstance->Montage_Play(SetToEquip->AnimationData->Equip_TPP);
				}
			}
		}
	}
}

void UEquipmentComponent::UnequipSet_Internal(bool bUnequipTemporarySet, bool bUnequipTemporarily)
{
	if (!GetOwner() || !GetWorld())
	{
		return;
	}

	// Determine which equipment handle to use to unequip the desired set.
	FEquipmentSetHandle& TargetHandle = bUnequipTemporarySet ? TemporarilyEquippedSetHandle : EquippedSetHandle;


	// Remove or disable the unequipped set's ability set on the server.
	if (GetOwner()->HasAuthority() && TargetHandle.GrantedASC)
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


	// Destroy the unequipped set's spawned actors (i.e. pieces).
	for (AEquipmentActor* EquipmentActor : TargetHandle.SpawnedEquipmentActors)
	{
		EquipmentActor->OnUnequip();
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
