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
 * Contains a map for native actions and another map for ability actions. Ability actions are automatically bound to
 * abilities with matching input tags, while native actions must be manually bound (usually to handler functions).
 *
 * Because input actions are in-editor assets, they need to be loaded or defined with references in the editor to be
 * used in C++. This mapping compartmentalizes the editor-defined references into an independent data asset and allows
 * input actions to be referenced in C++ easily using tags rather than object pointers.
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UCrashInputActionMapping : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Searches the native input action mapping for the specified input tag and returns the input action to which it
	 * maps. Returns nullptr if the tag was not found. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

	/** Searches the ability input action mapping for the specified input tag and returns the input action to which it
	 * maps. Returns nullptr if the tag was not found. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

public:

	/** A mapping of input tags to native input actions which must be manually bound. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCrashInputAction> NativeInputActions;

	/** A mapping of input tags to ability input actions that are bound automatically to abilities with matching
	 * input tags. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCrashInputAction> AbilityInputActions;
};
