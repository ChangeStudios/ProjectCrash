// Copyright Samuel Reitich 2024.


#include "AbilitySystem/Abilities/Generic/GA_Death.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "GameFramework/GameModes/CrashGameModeBase.h"
#include "Kismet/GameplayStatics.h"

UGA_Death::UGA_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = CrashGameplayTags::TAG_Event_Ability_Generic_Death;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo_Checked();

	// Cancel ongoing abilities, unless they shouldn't be cancelled by avatar death.
	FGameplayTagContainer IgnoreAbilitiesWithTags;
	IgnoreAbilitiesWithTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_PersistsThroughAvatarDestruction);

	ASC->CancelAbilities(nullptr, &IgnoreAbilitiesWithTags, this);

	AGameModeBase* GM = UGameplayStatics::GetGameMode(GetWorld());
	ACrashGameModeBase* CrashGM = Cast<ACrashGameModeBase>(GM);

	// Unpossess the player from the dying character.
	if (APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (Pawn->Controller)
		{
			Pawn->Controller->UnPossess();
		}
	}

	// Determine the duration of this death.
	const float DeathDuration = CrashGM ? CrashGM->DeathDuration : DefaultDeathDuration;

	// Start the death on the game mode.
	if (CrashGM)
	{
		CrashGM->StartDeath(GetActorInfo().PlayerController->Player);
	}

	// Set a timer to end this ability after DeathDuration.
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, FTimerDelegate::CreateLambda([this, ActorInfo, &ActivationInfo]
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, ActivationInfo, true, false);
	}), DeathDuration, false);
}

void UGA_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Finish the death on the game mode.
	if (ACrashGameModeBase* CrashGM = Cast<ACrashGameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		CrashGM->FinishDeath(GetActorInfo().PlayerController->Player);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
