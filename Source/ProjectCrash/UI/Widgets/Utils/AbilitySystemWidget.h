// Copyright Samuel Reitich 2024.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "AbilitySystemWidget.generated.h"

class UCrashAbilitySystemComponent;

/** Delegate for broadcasting when this widget has bound to its owner's ASC. */
DECLARE_MULTICAST_DELEGATE(FASCReadySignature);

/**
 * A widget that is bound to its owner's ability system component and registers to its delegates.
 */
UCLASS(Abstract, Blueprintable)
class PROJECTCRASH_API UAbilitySystemWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	// Initialization.

public:

	/** Binds this widget to its owner's ASC. */
	virtual void NativeConstruct() override;

	/** Broadcast when this widget has bound to its owner's ASC. This is necessary because clients will create their
	 * interface (including this widget) before their ASC has been initialized. */
	FASCReadySignature ASCReadyDelegate;



	// UI updates.

protected:

	/** Called when a new gameplay ability is granted to this widget's owner's ASC. */
	UFUNCTION()
	virtual void OnAbilityGranted(const FGameplayAbilitySpec& GrantedAbilitySpec);

	/** Called when a gameplay ability is removed from this widget's owner's ASC. */
	UFUNCTION()
	virtual void OnAbilityRemoved(const FGameplayAbilitySpec& RemovedAbilitySpec);



	// Utils.

protected:

	/** Attempts to retrieve and cache the new player state's owning ASC. */
	UFUNCTION()
	void OnPlayerStateChanged();

	/** This widget's owning player's ASC, cached for convenience. */
	UPROPERTY(BlueprintReadOnly)
	UCrashAbilitySystemComponent* OwningASC;
};
