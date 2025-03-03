// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/SkeletalMeshComponent.h"
#include "EquipmentMeshComponent.generated.h"

/**
 * The perspective of an equipment mesh. Separate equipment meshes, animations, and VFX are used for first- and
 * third-person.
 */
UENUM()
enum class EEquipmentPerspective : uint8
{
	FirstPerson,
	ThirdPerson
};



/**
 * A skeletal mesh component used for equipment in a character. Manages perspective visibility, first-person rendering,
 * and tag-based equipment management.
 *
 * This component should only be used in actors of type CrashCharacter, and should be attached to one of the two
 * character meshes.
 */
UCLASS(ClassGroup = "Rendering", Meta = (BlueprintSpawnableComponent))
class PROJECTCRASH_API UEquipmentMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:

	/** Syncs this component's visibility with its outer character mesh and enables first-person depth rendering if
	 * necessary. */
	virtual void BeginPlay() override;

	/** The perspective with which this mesh is used. */
	EEquipmentPerspective GetEquipmentPerspective() const { return Perspective; }

protected:

	/** The perspective with which this mesh is used (i.e. which character mesh this component is attached to). */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	EEquipmentPerspective Perspective = EEquipmentPerspective::ThirdPerson;

	/** The set of equipment to which this mesh belongs (e.g. Equipment.Knight.SwordAndShield). */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (Categories = "Equipment"))
	FGameplayTag EquipmentSet;

	/** The specific equipment which this mesh represents (e.g. Equipment.Knight.SwordAndShield.Sword). */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment", Meta = (Categories = "Equipment"))
	FGameplayTag Equipment;
};
