// Copyright Samuel Reitich 2024.

#pragma once

#include "CrashInputActionMapping.h"
#include "GameplayTagContainer.h"

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "CrashInputComponent.generated.h"

class UCrashInputActionMapping;

/**
 * An input component that uses input action mappings to bind handler functions or abilities to input actions.
 */
UCLASS(Config = Input)
class PROJECTCRASH_API UCrashInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

	// Input.

public:

	/**
	 * Binds the given handler function to the input action mapped to by the given input tag in the specified action
	 * mapping.
	 *
	 * @param ActionMapping		A mapping of input tags to input actions that provides the reference to the input
	 *							action to bind to. Must contain an entry for InputTag.
	 * @param InputTag			The tag used to retrieve the input action to bind to. Must have an entry in
	 *							ActionMapping.
	 * @param TriggerEvent		The trigger type to use for this binding. This is action-specific.
	 * @param Object			The object binding this action (i.e. where the handler function is defined).
	 * @param HandlerFunc		The handler function to bind this input action to.
	 * @param bLogIfFailed		Log a debug message if the given input action mapping does not contain an entry for
	 *							InputTag.
	 */
	template<class UserClass, typename FuncType>
	void BindInputAction(const UCrashInputActionMapping* ActionMapping, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType HandlerFunc, bool bLogIfFailed = true);



	// Utilities.

public:

	/** Returns the given actor's CrashInputComponent if it has one. Returns a nullptr otherwise. */
	UFUNCTION(BlueprintPure, Category = "Input")
	FORCEINLINE static UCrashInputComponent* FindCrashInputComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<UCrashInputComponent>() : nullptr);
	}
};


template<class UserClass, typename FuncType>
void UCrashInputComponent::BindInputAction(const UCrashInputActionMapping* ActionMapping, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType HandlerFunc, bool bLogIfFailed = true)
{
	check(ActionMapping);

	// Try to retrieve the input action to bind to using the given input tag.
	if (const UInputAction* ActionToBind = ActionMapping->FindsInputActionForTag(InputTag, bLogIfFailed))
	{
		// Bind the given handler function to the input action.
		BindAction(ActionToBind, TriggerEvent, Object, HandlerFunc);
	}
}