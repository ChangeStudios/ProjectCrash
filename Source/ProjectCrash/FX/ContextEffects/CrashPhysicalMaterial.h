// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CrashPhysicalMaterial.generated.h"

class USoundBase;
class UNiagaraSystem;

/**
 * Effects spawned in response to a certain physical effect (footstep, landing, etc.).
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FPhysicalMaterialEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> Effect;
};



/**
 * Extends physical material to provide data for triggering material-specific visual and audio effects.
 */
UCLASS()
class PROJECTCRASH_API UCrashPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()

public:

	/** Map of effects that will be spawned for corresponding physical events. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", Meta = (Categories = "Event.PhysicalMaterial"))
	TMap<FGameplayTag, FPhysicalMaterialEffect> Effects;

public:

	/**
	 * Triggers effects for the corresponding physical material event.
	 *
	 * Effects are spawned at the given socket's location on the given component. If no socket is given, effects will
	 * be spawned at the hit location. Otherwise, effects will be thrown out. Location and
	 * rotation will be offset by the given parameters.
	 */
	UFUNCTION(BlueprintCallable, Category = PhysicalMaterial)
	static void HandlePhysicalMaterialEvent
	(
		UPARAM(Meta = (Categories = "Event.PhysicalMaterial"))
		FGameplayTag Event,
		const FName TriggeringSocket,
		USceneComponent* TriggeringComponent,
		const FVector LocationOffset,
		const FRotator RotationOffset,
		const bool bHitSuccess,
		UCrashPhysicalMaterial* PhysicalMaterial,
		const FHitResult HitResult,
		const FVector VFXScale = FVector(1.0f),
		const float VolumeMultiplier = 1.0f,
		const float PitchMultiplier = 1.0f
	);
};
