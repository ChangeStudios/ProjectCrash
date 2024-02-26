// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrashAbilitySystemComponent.generated.h"

/** Delegate used to broadcast the Death event and communicate in important information. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeathEventSignature, const FDeathData&, DeathData);

/**
 * The ability system component used for this project.
 *
 * This project's GAS implementation (like any implementation of GAS) is complex and may be difficult to understand
 * purely from inline documentation. If you'd like a more digestible explanation of how this framework functions, view
 * this document detailing the implementation this system from a higher level:
 *
 *		https://docs.google.com/document/d/1lrocajswgfGHrTl-TFuM-Iw5N444_QWDiKfILris98c/edit?usp=sharing
 */
UCLASS()
class PROJECTCRASH_API UCrashAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Registers this ASC with the global ability system. */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;



	// Uninitialization.

public:

	/** Unregisters this ASC from the global ability system. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	// Utilities.

public:

	/** Returns the actor currently acting as this ASC's avatar. Blueprint-exposed wrapper for GetAvatarActor(). */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability System|Utilities", Meta = (DisplayName = "Get Avatar Actor"))
	AActor* K2_GetAvatarActor() { return GetAvatarActor(); };



	// Animation.

public:

	/**
	 * Plays the given montage on this ASC's avatar's first-person mesh, if the avatar is a ACrashCharacterBase.
	 * Otherwise, returns -1.0.
	 *
	 * This does not affect the ability system's animation data, such as LocalAnimMontageInfo. The ability system's
	 * animation data is only affected by third-person animations, since first-person animations are rarely relevant to
	 * anyone besides the local client.
	 */
	float PlayFirstPersonMontage(UGameplayAbility* AnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f);

protected:

	/** Called when a prediction key that played a first-person montage is rejected. */
	void OnFirstPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage);

	/** Called when a prediction key that played a third-person montage is rejected. */
	void OnThirdPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage);
	


	// Death.

public:

	/** Delegate broadcast when a Death ability is activated to communicate death data. */
	UPROPERTY()
	FDeathEventSignature DeathEventDelegate;

};
