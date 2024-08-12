// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "EquipmentList.h"
#include "Components/PawnComponent.h"
#include "EquipmentComponent.generated.h"

class UEquipmentDefinition;
class UEquipmentInstance;

/**
 * Allows the owning pawn to equip equipment which can be accessed and managed via this component.
 *
 * The pawn (or its player state) does not need an inventory component to use equipment, but they should have on if they
 * want to manage the items from which equipment is created (i.e. inventory items with the "equippable" trait).
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UEquipmentComponent : public UPawnComponent
{
	GENERATED_BODY()

#if WITH_GAMEPLAY_DEBUGGER
	friend FGameplayDebuggerCategory_Equipment;
#endif // WITH_GAMEPLAY_DEBUGGER

	// Construction.

public:

	UEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

public:

	/** Unequips all equipment when this component is uninitialized. */
	virtual void UninitializeComponent() override;



	/* Replication. Equipment instances are replicated as sub-objects of the equipment component from which they are
	 * equipped. Because equipment only exists while equipped (spawned on equip, destroyed on unequip), equipment is
	 * managed and replicated by the same equipment component for its entire lifetime. */

public:

	/** Registers each equipment instance equipped by this component as a replicated sub-object. */
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	/** Registers any equipment instances that were equipped before this component began replicating as replicated
	 * sub-objects when replication starts. */
	virtual void ReadyForReplication() override;



	// Equipment management.

public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	UEquipmentInstance* EquipItem
	(
		UPARAM(DisplayName = "Item to Equip")
		UEquipmentDefinition* EquipmentDefinition
	);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	void UnequipItem
	(
		UPARAM(DisplayName = "Item to Unequip")
		UEquipmentInstance* EquipmentInstance
	);



	// Internals.

private:

	/** Current equipment. Equipment only exists while equipped, so all equipment in this list is equipped to this
	 * component's owning pawn. */
	UPROPERTY(Replicated)
	FEquipmentList EquipmentList;



	
	// Utils.

public:

	/** All equipment currently equipped by this component's owning pawn. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<UEquipmentInstance*> GetAllEquipment() const;

	/** All equipped instances of the given equipment currently equipped by this component's owning pawn. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<UEquipmentInstance*> GetAllEquipmentOfDefinition(UEquipmentDefinition* EquipmentToFind) const;

	/** All equipped instances of the specified instance type. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	TArray<UEquipmentInstance*> GetAllEquipmentOfType(TSubclassOf<UEquipmentInstance> EquipmentInstanceType) const;

	/** Returns the first equipped instance of the given equipment. Returns null if none can be found. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UEquipmentInstance* GetFirstEquipmentByDefinition(UEquipmentDefinition* EquipmentToFind) const;

	/** Returns the first equipped instance of the specified instance type. Returns null if none can be found. */
	UFUNCTION(BlueprintPure, Category = "Equipment", Meta = (DeterminesOutputType = "EquipmentInstanceType"))
	UEquipmentInstance* GetFirstEquipmentByType(TSubclassOf<UEquipmentInstance> EquipmentInstanceType) const;

	/** Templated version of GetFirstInstanceByType. */
	template <typename T>
	T* GetFirstEquipmentInstanceOfType()
	{
		return (T*)GetFirstEquipmentByType(T::StaticClass());
	}

	/** Retrieves the given pawn's inventory component, if it has one. Returns null otherwise. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	static UEquipmentComponent* FindEquipmentComponent(const APawn* Pawn) { return Pawn->FindComponentByClass<UEquipmentComponent>(); }
};
