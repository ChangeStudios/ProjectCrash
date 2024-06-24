// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashInputActionMapping.generated.h"

class UInputAction;

/**
 * A structure mapping an input tag to an input action. This is used to reference input action objects using tags.
 */
USTRUCT(BlueprintType)
struct FCrashInputAction
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;

	/** This struct can be equivalent to an input action if it's wrapping that input action. */
	bool operator==(const UInputAction* Other) const
	{
		return InputAction == Other;
	}
};



/**
 * A map of input tags to input actions. This mapping is used to bind events (e.g. abilities) to input actions, using
 * tags to reference those actions.
 *
 * Contains one map for native actions and one map for ability actions. Ability actions are automatically bound to
 * abilities with matching input tags, while native actions must be manually bound to handler functions.
 *
 * Because input actions are assets, to use them in C++, we either need to manually set references to each object in the
 * editor or load them from the Content directory. This mapping compartmentalizes a collection of editor-defined input
 * actions into an independent data asset, so we don't have to repeatedly reference/load each input action individually.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UCrashInputActionMapping : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Searches the native input action mapping for the specified input tag and returns the input action to which it
	 * maps. Returns null if the tag was not found. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** Searches the ability input action mapping for the specified input tag and returns the input action to which it
	 * maps. Returns null if the tag was not found. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:

	/** A mapping of input tags to native input actions. Native actions must be manually bound to handler functions.
	 * See UInputHandlerComponentBase. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCrashInputAction> NativeInputActions;

	/** A mapping of input tags to ability input actions. Ability actions are automatically bound to abilities with
	 * matching input tags. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCrashInputAction> AbilityInputActions;
};
