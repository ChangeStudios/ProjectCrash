// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CharacterWithAbilities.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AIController.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif	// WITH_EDITOR

#define LOCTEXT_NAMESPACE "CharacterWithAbilities"

ACharacterWithAbilities::ACharacterWithAbilities(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Construct the ASC.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	MinNetUpdateFrequency = 2.0f;
	NetUpdateFrequency = 100.0f;
}

UAbilitySystemComponent* ACharacterWithAbilities::GetAbilitySystemComponent() const
{
	// Use this character's self-contained ASC.
	return AbilitySystemComponent;
}

#if WITH_EDITOR
EDataValidationResult ACharacterWithAbilities::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// If this character has a controller, make sure it won't create a player state.
	if (AIControllerClass)
	{
		// This character cannot be controlled by a player controller.
		if (AIControllerClass->IsChildOf(APlayerController::StaticClass()))
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(LOCTEXT("CharacterWillHavePlayerState", "Character uses a player controller. This will spawn a player state for this character, which will interfere with its self-contained ASC. Use a non-player controller instead."));
		}

		// This character cannot be controlled by an AI controller that wants a player state.
		else if (AIControllerClass->IsChildOf(AAIController::StaticClass()))
		{
			if (AAIController* ControllerCDO = AIControllerClass->GetDefaultObject<AAIController>())
			{
				if (ControllerCDO->bWantsPlayerState)
				{
					Result = EDataValidationResult::Invalid;
					Context.AddError(FText::Format(LOCTEXT("CharacterWillHavePlayerState", "Character uses controller {0}, which wants a player state. This will interfere with this character's self-contained ASC. Use an AI controller that does not want a player state instead."), FText::FromString(GetNameSafe(AIControllerClass))));
				}
			}
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE