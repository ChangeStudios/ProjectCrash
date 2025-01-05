// Copyright Samuel Reitich. All rights reserved.


#include "Effects/ContextEffects/AnimNotify_PhysicalMaterialEvent.h"

#include "CrashPhysicalMaterial.h"


UAnimNotify_PhysicalMaterialEvent::UAnimNotify_PhysicalMaterialEvent() :
	LocationOffset(FVector::ZeroVector),
	RotationOffset(FRotator::ZeroRotator),
	bAttached(true),
	bPerspectiveBased(true)
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = true;

	NotifyColor = FColor(0, 255, 128, 255);
#endif

	// Initialize the fallback physical material if it's already loaded.
	DefaultPhysicalMaterial = DefaultPhysicalMaterialPath.Get();
}

void UAnimNotify_PhysicalMaterialEvent::PostLoad()
{
	Super::PostLoad();

	// Load the fallback physical material.
	if (DefaultPhysicalMaterialPath.IsNull() && DefaultPhysicalMaterialPath.ToSoftObjectPath().IsValid())
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
		// Ignore physical material events if we're playing a full-body animation.
		if (!ShouldAnimationTriggerNotify(MeshComp->GetAnimInstance(), Animation))
		{
			return;
		}

		// Make sure this notify's mesh component and its owning actor are valid.
		if (AActor* OwningActor = MeshComp->GetOwner())
		{
			if (const APawn* Pawn = Cast<APawn>(OwningActor))
			{
				// Don't play effects triggered by invisible meshes if this is a perspective-based effect.
				if (bPerspectiveBased && !MeshComp->IsVisible())
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

bool UAnimNotify_PhysicalMaterialEvent::ShouldAnimationTriggerNotify(UAnimInstance* AnimInstance, UAnimSequenceBase* Animation)
{
	/* Physical material events are ignored if there is a full-body montage playing. For example, if the player is
	 * dashing forwards while on the ground, we wouldn't want to trigger their footsteps. But if the player is doing
	 * something with their arms, or firing a weapon, we still want to play their footsteps. */
	return (AnimInstance->GetSlotMontageGlobalWeight(FName("FullBody")) == 0.0f);
}
