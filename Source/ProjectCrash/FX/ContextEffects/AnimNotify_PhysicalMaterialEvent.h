// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CrashPhysicalMaterial.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PhysicalMaterialEvent.generated.h"

/**
 * Parameters for spawning VFX from physical material events.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FPhysicalMaterialEventVFXSettings
{
	GENERATED_BODY()

	/** Scale with which to spawn the particle system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FX)
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};



/**
 * Parameters for playing audio from physical material events.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FPhysicalMaterialEventAudioSettings
{
	GENERATED_BODY()

	/** Volume multiplier for the audio to be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float VolumeMultiplier = 1.0f;

	/** Pitch multiplier for the audio to be played. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound)
	float PitchMultiplier = 1.0f;
};



/**
 * Parameters for performing traces for physical material events.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FPhysicalMaterialEventTraceSettings
{
	GENERATED_BODY()

	/** Channel to perform the trace in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility;

	/** Vector offset from event's root location. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	FVector EndTraceLocationOffset = FVector::ZeroVector;

	/** Whether to ignore this actor when getting trace result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trace)
	bool bIgnoreActor = true;
};



/**
 * Triggers a physical material event with given parameters. Physical material events are used for events like
 * footsteps and landing. Materials of the CrashPhysicalMaterial class define which effects to spawn, depending on
 * the event.
 */
UCLASS(Const, HideCategories = Object, CollapseCategories, Config = Game, Meta = (DisplayName = "Trigger Physical Material Event"))
class PROJECTCRASH_API UAnimNotify_PhysicalMaterialEvent : public UAnimNotify
{
	GENERATED_BODY()

	// Animation notify.

public:

	/** Loads the default physical material when this notify is loaded. */
	virtual void PostLoad() override;

	/** Uses Event as the notify name. */
	virtual FString GetNotifyName_Implementation() const override;

	/** Performs a trace to find a physical material. If one is found, handles this notify's specified material event
	 * using that physical material and this notify's parameters. */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;



	// Notify parameters.

private:

	/** Physical material used if one cannot be found with the trace. */
	UPROPERTY()
	UCrashPhysicalMaterial* DefaultPhysicalMaterial;

	/** Path to the fallback physical material. */
	UPROPERTY(Config)
	TSoftObjectPtr<UCrashPhysicalMaterial> DefaultPhysicalMaterialPath;

public:

	/** Material event to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (Categories = "Event.PhysicalMaterial", ExposeOnSpawn = true))
	FGameplayTag Event;

	/** Location offset from the socket location (if attached). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (ExposeOnSpawn = true))
	FVector LocationOffset = FVector::ZeroVector;

	/** Rotation offset from socket rotation (if attached). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (ExposeOnSpawn = true))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Parameters for spawning the event's VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (ExposeOnSpawn = true))
	FPhysicalMaterialEventVFXSettings VFXProperties;

	/** Parameters for playing the event's audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (ExposeOnSpawn = true))
	FPhysicalMaterialEventAudioSettings AudioProperties;

	/** Whether to attach this event's spawned objects to a bone/socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Properties", Meta = (ExposeOnSpawn = true))
	bool bAttached = true;

	/** Name of the socket to attach to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Properties", Meta = (ExposeOnSpawn = true, EditCondition = "bAttached"))
	FName SocketName;

	/** Parameters for the line trace performed by this event to find the target physical material. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", Meta = (ExposeOnSpawn = true))
	FPhysicalMaterialEventTraceSettings TraceProperties;
};
