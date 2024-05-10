// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

UEquipmentComponent::UEquipmentComponent()
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
}

void UEquipmentComponent::EquipSet(UEquipmentSet* SetToEquip)
{
}

void UEquipmentComponent::TemporarilyEquipSet(UEquipmentSet* SetToTemporarilyEquip)
{
}

bool UEquipmentComponent::UnequipTemporarySet()
{
	return false;
}

void UEquipmentComponent::EquipSet_Internal(UEquipmentSet* SetToEquip)
{
}

void UEquipmentComponent::UnequipSet_Internal(bool bUnequipTemporarySet, bool bUnequipTemporarily)
{
}

void UEquipmentComponent::OnRep_EquippedSet(UEquipmentSet* PreviouslyEquippedSet)
{
}

void UEquipmentComponent::OnRep_TemporarilyEquippedSet(UEquipmentSet* PreviouslyTemporarilyEquippedSet)
{
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, EquippedSet, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEquipmentComponent, TemporarilyEquippedSet, COND_None, REPNOTIFY_Always);
}
