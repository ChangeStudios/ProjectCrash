// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

UInventoryItemInstance::UInventoryItemInstance()
{
}

#if UE_WITH_IRIS
void UInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object.
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UInventoryItemInstance::AddStatTags(FGameplayTag Tag, int32 Count)
{
	StatTags.AddTags(Tag, Count);
}

void UInventoryItemInstance::RemoveStatTags(FGameplayTag Tag, int32 Count)
{
	StatTags.RemoveTags(Tag, Count);
}

int32 UInventoryItemInstance::GetStatTagCount(FGameplayTag Tag) const
{
	return StatTags.GetTagCount(Tag);
}

bool UInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInventoryItemInstance, StatTags);
}