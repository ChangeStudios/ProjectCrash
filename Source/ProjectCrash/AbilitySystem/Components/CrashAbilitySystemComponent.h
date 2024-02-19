// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CrashAbilitySystemComponent.generated.h"

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

};
