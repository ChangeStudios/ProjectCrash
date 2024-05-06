// Copyright Samuel Reitich 2024.


#include "FX/ContextEffects/CrashPhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


void UCrashPhysicalMaterial::HandlePhysicalMaterialEvent(FGameplayTag Event, const FName TriggeringSocket,
	USceneComponent* TriggeringComponent, const FVector LocationOffset, const FRotator RotationOffset,
	const bool bHitSuccess, UCrashPhysicalMaterial* PhysicalMaterial, const FHitResult HitResult,
	const FVector VFXScale, const float VolumeMultiplier, const float PitchMultiplier)
{
	if (!PhysicalMaterial || !TriggeringComponent)
	{
		return;
	}

	if (const FPhysicalMaterialEffect* MaterialEffect = PhysicalMaterial->Effects.Find(Event))
	{
		// Use the triggering component's transform by default.
		FVector SpawnLocation = TriggeringComponent->GetComponentLocation();
		FRotator SpawnRotation = TriggeringComponent->GetComponentRotation();

		// If there is a valid socket, use the socket for the location.
		if (!TriggeringSocket.IsNone())
		{
			SpawnLocation = TriggeringComponent->GetSocketLocation(TriggeringSocket);
		}
		// If there is no socket but a valid hit, use the hit location.
		else if (bHitSuccess)
		{
			SpawnLocation = HitResult.Location;
		}

		// Spawn the system at the target location.
		if (MaterialEffect->Effect)
		{
			UE_LOG(LogTemp, Error, TEXT("Spawned effect %s"), *GetNameSafe(MaterialEffect->Effect));
			UNiagaraFunctionLibrary::SpawnSystemAtLocation
			(
				TriggeringComponent,
				MaterialEffect->Effect,
				SpawnLocation + LocationOffset,
				SpawnRotation + RotationOffset,
				VFXScale
			);
		}

		// Play the sound at the target location.
		if (MaterialEffect->Sound)
		{
			UGameplayStatics::PlaySoundAtLocation
			(
				TriggeringComponent,
				MaterialEffect->Sound,
				SpawnLocation + LocationOffset,
				SpawnRotation + RotationOffset,
				VolumeMultiplier,
				PitchMultiplier,
				0,
				MaterialEffect->Sound->AttenuationSettings,
				nullptr,
				TriggeringComponent->GetOwner(),
				nullptr
			);
		}
	}
}
