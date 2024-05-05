// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CrashPhysicalMaterial.generated.h"

class USoundBase;
class UNiagaraSystem;

/**
 * Extends physical material to provide data for triggering material-specific visual and audio effects.
 */
UCLASS()
class PROJECTCRASH_API UCrashPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

public:

	/** Sound that will be played when a character takes a step on this material. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Footsteps")
	TObjectPtr<USoundBase> FootstepSound = nullptr;

	/** Visual effect that will be spawn when a character takes a step on this material. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Footsteps")
	TObjectPtr<UNiagaraSystem> FootstepEffect;



	/** Sound that will be played when a character lands on this material. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Landing")
	TObjectPtr<USoundBase> LandingSound = nullptr;

	/** Visual effect that will be spawn when a character lands on this material. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Landing")
	TObjectPtr<UNiagaraSystem> LandingEffect;
};
