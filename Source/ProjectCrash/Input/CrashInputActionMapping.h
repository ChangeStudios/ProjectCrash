// Copyright Samuel Reitich 2024.

#pragma once

#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrashInputActionMapping.generated.h"

/** Native gameplay tags relevant to this class. */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Look_Mouse);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Look_Stick);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InputTag_Move);

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
};


/**
 * A map of input tags to input actions. This mapping is used to bind events (e.g. abilities) to input actions, using
 * tags to reference those actions.
 *
 * Because input actions are in-editor assets, they need to be loaded or defined with references in the editor to be
 * used in C++. This map compartmentalizes the editor-defined references into an independent data asset and allows
 * input actions to be referenced in C++ easily using tags.
 *
 */
UCLASS(BlueprintType, Const)
class PROJECTCRASH_API UCrashInputActionMapping : public UDataAsset
{
	GENERATED_BODY()

public:

	/** Searches this mapping for the specified input tag and returns the input action to which it maps. Returns
	 * nullptr if the tag was not found. */
	UFUNCTION(BlueprintCallable, Category = "Input")
	const UInputAction* FindsInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;

protected:

	/** A mapping of input tags to their corresponding input actions. */
	UPROPERTY(EditDefaultsOnly)
	TArray<FCrashInputAction> InputActions;
};
