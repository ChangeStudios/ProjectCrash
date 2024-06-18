// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CharacterWithAbilities.h"

#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AIController.h"
#include "GameFramework/CrashLogging.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

#if WITH_EDITOR

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

void ACharacterWithAbilities::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Make sure this character's AI controller doesn't request a player state.
	if (AIControllerClass)
	{
		// This character cannot be AI-controlled by a player controller. This would create a player state for it.
		if (AIControllerClass->IsChildOf(APlayerController::StaticClass()))
		{
			UE_LOG(LogCrash, Error, TEXT("Character [%s] has player controller [%s] set as its AI controller. This will create a player state for this character, which will interfere with its self-contained ASC."),
				*GetNameSafe(this),
				*GetNameSafe(AIControllerClass));

			static const FText Message = NSLOCTEXT("CharacterWithAbilities", "PlayerStateRequestedError", "uses a player controller as its AI controller class. This means it will be given a player state. This character possesses its own ASC and will not function properly with a player state.");
			static const FName CharacterWithAbilitiesMessageLogName = TEXT("CharacterWithAbilities");

			FMessageLog(CharacterWithAbilitiesMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(CharacterWithAbilitiesMessageLogName).Open();
		}

		// This character cannot be AI-controlled by a controller that wants a player state.
		else if (AIControllerClass->IsChildOf(AAIController::StaticClass()))
		{
			if (AAIController* ControllerCDO = AIControllerClass->GetDefaultObject<AAIController>())
			{
				if (ControllerCDO->bWantsPlayerState)
				{
					UE_LOG(LogCrash, Error, TEXT("Character [%s] has controller [%s] set as its AI controller, which wants a player state. This character cannot use a player state, as it will interfere with its self-contained ASC."),
						*GetNameSafe(this),
						*GetNameSafe(AIControllerClass));

					static const FText Message = NSLOCTEXT("CharacterWithAbilities", "PlayerStateRequestedError", "uses an AI controller that wants a player state. This character possesses its own ASC and will not function properly with a player state.");
					static const FName CharacterWithAbilitiesMessageLogName = TEXT("CharacterWithAbilities");

					FMessageLog(CharacterWithAbilitiesMessageLogName).Error()
						->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
						->AddToken(FTextToken::Create(Message));

					FMessageLog(CharacterWithAbilitiesMessageLogName).Open();
				}
			}
		}
	}
}

#endif // WITH_EDITOR

UAbilitySystemComponent* ACharacterWithAbilities::GetAbilitySystemComponent() const
{
	// Use this character's self-contained ASC.
	return AbilitySystemComponent;
}
