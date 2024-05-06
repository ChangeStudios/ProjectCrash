// Copyright Samuel Reitich 2024.


#include "FX/ContextEffects/AnimNotify_PhysicalMaterialEvent.h"

#include "CrashPhysicalMaterial.h"


void UAnimNotify_PhysicalMaterialEvent::PostLoad()
{
	Super::PostLoad();

	// Load the fallback physical material.
	if (DefaultPhysicalMaterialPath.IsValid())
	{
		DefaultPhysicalMaterial = DefaultPhysicalMaterialPath.LoadSynchronous();
	}
}

FString UAnimNotify_PhysicalMaterialEvent::GetNotifyName_Implementation() const
{
	// Use Event (i.e. the event tag name) as this notify's name, if it's been set.
	if (Event.IsValid())
	{
		return Event.ToString();
	}

	return Super::GetNotifyName_Implementation();
}

void UAnimNotify_PhysicalMaterialEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// Make sure this notify's mesh component and its owning actor are valid.
		if (AActor* OwningActor = MeshComp->GetOwner())
		{
			if (const APawn* Pawn = Cast<APawn>(OwningActor))
			{
				// Don't play remote-only notifies locally.
				if (MeshComp->bOwnerNoSee && Pawn->IsLocallyControlled())
				{
					return;
				}

				// Don't play local-only notifies remotely.
				if (MeshComp->bOnlyOwnerSee && !Pawn->IsLocallyControlled())
				{
					return;
				}
			}

			// Generate data for the trace.
			bool bHitSuccess = false;
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.bReturnPhysicalMaterial = true;
			UCrashPhysicalMaterial* PhysicalMaterial = nullptr;

			if (TraceProperties.bIgnoreActor)
			{
				// Ignore the notify's owning actor, if desired.
				QueryParams.AddIgnoredActor(OwningActor);
			}

			// Perform the line trace.
			if (const UWorld* World = OwningActor->GetWorld())
			{
				const FVector TraceStart = bAttached ? MeshComp->GetSocketLocation(SocketName) : MeshComp->GetComponentLocation();
				const FVector TraceEnd = TraceStart + TraceProperties.EndTraceLocationOffset;

				bHitSuccess = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceProperties.TraceChannel, QueryParams, FCollisionResponseParams::DefaultResponseParam);

				// Cache the physics material as a CrashPhysicsMaterial if one was hit.
				if (bHitSuccess && HitResult.PhysMaterial.Get())
				{
					PhysicalMaterial = Cast<UCrashPhysicalMaterial>(HitResult.PhysMaterial.Get());
				}
			}

			// Handle the physical material event.
			UCrashPhysicalMaterial::HandlePhysicalMaterialEvent
			(
				Event,
				bAttached ? SocketName : FName("None"),
				MeshComp,
				LocationOffset,
				RotationOffset,
				bHitSuccess,
				IsValid(PhysicalMaterial) ? PhysicalMaterial : DefaultPhysicalMaterial,
				HitResult,
				VFXProperties.Scale,
				AudioProperties.VolumeMultiplier,
				AudioProperties.PitchMultiplier
			);
		}
	}
}
