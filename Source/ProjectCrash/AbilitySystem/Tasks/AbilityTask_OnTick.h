// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_OnTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTickTaskDelegate, float, DeltaTime);

/**
 * Ability task that can hook gameplay abilities into the Tick process. Will fire a callback each tim e
 */
UCLASS()
class PROJECTCRASH_API UAbilityTask_OnTick : public UAbilityTask
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	UAbilityTask_OnTick(const FObjectInitializer& ObjectInitializer);
	
	/** Fires a callback every tick until this task ends. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_OnTick* OnAbilityTick(UGameplayAbility* OwningAbility, FName TaskInstanceName);



	// Ticking.

public:

	/** Broadcasts OnTick each time this task is ticked. */
	virtual void TickTask(float DeltaTime) override;

	/** Delegate broadcast each tick this task is active. */
	UPROPERTY(BlueprintAssignable)
	FOnTickTaskDelegate OnTick;



	// Internal variables.
		
protected:

	/** Time since the last tick. */
	float TaskDeltaTime;
};