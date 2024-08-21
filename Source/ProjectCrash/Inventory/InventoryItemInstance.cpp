// Copyright Samuel Reitich. All rights reserved.


#include "Inventory/InventoryItemInstance.h"

#include "Net/UnrealNetwork.h"
#include "Traits/InventoryItemTraitBase.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

UInventoryItemInstance::UInventoryItemInstance() :
	Owner(nullptr),
	ItemDefinition(nullptr)
{
}

void UInventoryItemInstance::InitItem(AActor* InOwner, TSubclassOf<UInventoryItemDefinition> InItemDefinition)
{
	check(InOwner);
	// Item definition should only be set once.
	check(ItemDefinition == nullptr);

	Owner = InOwner;
	ItemDefinition = InItemDefinition;

	// Initialize each of this item's traits.
	for (UInventoryItemTraitBase* Trait : GetDefault<UInventoryItemDefinition>(InItemDefinition)->Traits)
	{
		if (Trait != nullptr)
		{
			Trait->OnItemCreated(this);
		}
	}
}

void UInventoryItemInstance::BeginDestroy()
{
	// Uninitialize each of this item's traits.
	if (ItemDefinition)
	{
		for (UInventoryItemTraitBase* Trait : GetDefault<UInventoryItemDefinition>(ItemDefinition)->Traits)
		{
			if (Trait != nullptr)
			{
				Trait->OnItemDestroyed(this);
			}
		}
	}

	Super::BeginDestroy();
}

#if UE_WITH_IRIS
void UInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object.
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

const UInventoryItemTraitBase* UInventoryItemInstance::FindTraitByClass(TSubclassOf<UInventoryItemTraitBase> TraitClass) const
{
	if ((ItemDefinition != nullptr) && (TraitClass != nullptr))
	{
		// Get the desired trait through the item definition's CDO.
		return GetDefault<UInventoryItemDefinition>(ItemDefinition)->FindTraitByClass(TraitClass);
	}

	return nullptr;
}

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

	DOREPLIFETIME(UInventoryItemInstance, Owner);
	DOREPLIFETIME(UInventoryItemInstance, ItemDefinition);
	DOREPLIFETIME(UInventoryItemInstance, StatTags);
}