// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ModularCharacter.h"
#include "CrashCharacter.generated.h"

/**
 * Base modular character class for this project. Provides first- and third-person perspective functionality via
 * dual mesh components; provides a health component and an interface with the ability system; contains a pawn
 * extension component for coordinating modular actor components.
 *
 * New behavior should be implemented modularly via actor components; specifically, UPawnComponent. 
 */
UCLASS()
class PROJECTCRASH_API ACrashCharacter : public AModularCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	ACrashCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Mesh components.

public:

	/** Getter for the first-person mesh component. */
	FORCEINLINE USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Getter for the third-person mesh component. */
	FORCEINLINE USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }

private:

	/** Mesh component only visible in first-person. Determined by current camera mode. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	/** Mesh component only visible in third-person. Determined by current camera mode. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ThirdPersonMesh;
};
