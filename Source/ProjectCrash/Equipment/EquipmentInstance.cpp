// Copyright Samuel Reitich. All rights reserved.


#include "Equipment/EquipmentInstance.h"

#include "Net/UnrealNetwork.h"

UEquipmentInstance::UEquipmentInstance()
{
}

#if UE_WITH_IRIS
void UEquipmentInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UObject::RegisterReplicationFragments(Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

APawn* UEquipmentInstance::GetPawn() const
{
	return nullptr;
}

APawn* UEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	return nullptr;
}

void UEquipmentInstance::OnEquipped()
{
}

void UEquipmentInstance::OnUnequipped()
{
}

void UEquipmentInstance::SpawnEquipmentActors(const TArray<FEquipmentActorSpawnData>& ActorsToSpawn, bool bSpawnInFirstPerson)
{
}

void UEquipmentInstance::DestroyEquipmentActors()
{
}

UWorld* UEquipmentInstance::GetWorld() const
{
	return UObject::GetWorld();
}

void UEquipmentInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Instigator);
	DOREPLIFETIME(ThisClass, SpawnedActors);
}