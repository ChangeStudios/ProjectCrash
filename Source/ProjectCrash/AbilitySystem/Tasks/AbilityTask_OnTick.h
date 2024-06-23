// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_OnTick.generated.h"

/** Broadcasts each tick this task is active. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickTaskDelegate, float, DeltaTime);

/**
 * Hooks gameplay abilities into the tick process, allowing them to fire callbacks each tick while the task is active.
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_OnTick : public UAbilityTask
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UAbilityTask_OnTick(const FObjectInitializer& ObjectInitializer);



	// Task construction.

public:

	/** Fires a callback every tick until this task ends. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", Meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_OnTick* OnAbilityTick(UGameplayAbility* OwningAbility, FName TaskInstanceName);



	// Ticking.

public:

	/** Broadcasts OnTick each time this task is ticked. */
	virtual void TickTask(float DeltaTime) override;

	/** Delegate broadcast each tick this task is active. */
	UPROPERTY(BlueprintAssignable)
	FOnTickTaskDelegate OnTick;



	// Internals.
		
protected:

	/** Time since the last tick. */
	float TaskDeltaTime;
};