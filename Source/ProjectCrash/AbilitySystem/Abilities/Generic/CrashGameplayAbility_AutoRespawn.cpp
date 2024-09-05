// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/Generic/CrashGameplayAbility_AutoRespawn.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/AttributeSets/LivesAttributeSet.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CrashPlayerController.h"

UCrashGameplayAbility_AutoRespawn::UCrashGameplayAbility_AutoRespawn(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer),
	bIsListeningForReset(false),
	bShouldFinishReset(false)
{
    ActivationMethod = EAbilityActivationMethod::Passive;
	bServerRespectsRemoteAbilityCancellation = false;
	bRetriggerInstancedAbility = true;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// This ability should never be cancelled.
		AbilityTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_SurvivesDeath);
	}
}

void UCrashGameplayAbility_AutoRespawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ActivateOrStartListeningForDeath();
}

void UCrashGameplayAbility_AutoRespawn::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	StopListeningForDeath();
}

void UCrashGameplayAbility_AutoRespawn::OnNewAvatarSet()
{
	Super::OnNewAvatarSet();

	ActivateOrStartListeningForDeath();
}

void UCrashGameplayAbility_AutoRespawn::ActivateOrStartListeningForDeath()
{
	/* Sometimes we want to instantly reset our players without killing them (e.g. between rounds or when switching
	 * characters). When this happens, we want to skip the death sequence and just reset them. */
	if (!bIsListeningForReset)
	{
		// TODO: Start listening for reset.

		bIsListeningForReset = true;
	}

	// If the avatar is already dead, immediately start death logic.
	if (IsAvatarDeadOrDying())
	{
		OnDeathStarted(GetAvatarActorFromActorInfo());
	}
	// If the avatar is still alive, start listening for its death.
	else
	{
		StartListeningForDeath();
	}
}

void UCrashGameplayAbility_AutoRespawn::OnDeathStarted(AActor* DyingActor)
{
	StopListeningForDeath();

	ControllerToReset = GetControllerFromActorInfo();
	if (IsValid(ControllerToReset))
	{
		// Delay death logic until next tick to make sure conditionals are updated (e.g. lives have been decremented).
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]
		{
			/* If the player can respawn (e.g. has lives left), wait a specific duration, determined by GetRespawnTime,
			 * before resetting them. */
			if (CanRespawn())
			{
				// TODO: Broadcast death message with respawn time.
				const float RespawnTime = GetRespawnTime();

				// Start timer to respawn on the server.
				if (HasAuthority(&CurrentActivationInfo))
				{
					bShouldFinishReset = true;
					FTimerHandle TimerHandle;
					GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UCrashGameplayAbility_AutoRespawn::OnRespawnTimerEnd, RespawnTime, false);
				}
			}
			// If the player can't respawn, the ability handles what to do with them (e.g. making them a spectator).
			else
			{
				OnRespawnFailed();
			}
		});
	}
}

void UCrashGameplayAbility_AutoRespawn::FinishReset()
{
	// Prevent player from resetting multiple times (e.g. if a direct reset is requested during a respawn timer).
	bShouldFinishReset = false;

	if (IsValid(ControllerToReset))
	{
		// Restart the player.
		AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
		ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
		if (ensure(IsValid(CrashGM)))
		{
			CrashGM->RequestPlayerRestartNextTick(ControllerToReset, true);
		}

		// TODO: Broadcast respawn completed
	}
}

void UCrashGameplayAbility_AutoRespawn::StartListeningForDeath()
{
	// Make sure we aren't already listening for death.
	StopListeningForDeath();

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (IsValid(Avatar))
	{
		// Listen for if the avatar is destroyed before it dies. This triggers an instant reset, but shouldn't happen.
		BoundAvatar = Avatar;
		Avatar->OnEndPlay.AddDynamic(this, &UCrashGameplayAbility_AutoRespawn::OnAvatarEndPlay);

		// Listen for the avatar's death via the health component.
		BoundHealthComponent = UHealthComponent::FindHealthComponent(Avatar);
		if (IsValid(BoundHealthComponent))
		{
			BoundHealthComponent->DeathStartedDelegate.AddDynamic(this, &UCrashGameplayAbility_AutoRespawn::OnDeathStarted);
		}
	}
}

void UCrashGameplayAbility_AutoRespawn::StopListeningForDeath()
{
	// Clear avatar destruction delegate.
	if (IsValid(BoundAvatar))
	{
		BoundAvatar->OnEndPlay.RemoveDynamic(this, &UCrashGameplayAbility_AutoRespawn::OnAvatarEndPlay);
	}

	// Clear avatar death delegate.
	if (IsValid(BoundHealthComponent))
	{
		BoundHealthComponent->DeathStartedDelegate.RemoveDynamic(this, &UCrashGameplayAbility_AutoRespawn::OnDeathStarted);
	}

	// We can't clear the BoundAvatar yet; we still use it during the death sequence, after this function is called.
	BoundHealthComponent = nullptr;
}

void UCrashGameplayAbility_AutoRespawn::OnAvatarEndPlay(AActor* Avatar, EEndPlayReason::Type Reason)
{
	if (Reason == EEndPlayReason::Type::Destroyed)
	{
		if (HasAuthority(&CurrentActivationInfo))
		{
			ensureAlwaysMsgf(true, TEXT("Avatar [%s] destroyed before death without player reset!"), *GetNameSafe(Avatar));

			// Perform an instant reset. This should never happen; the ResetPlayer event should be used instead.
			ControllerToReset = GetControllerFromActorInfo();
			StopListeningForDeath();
			FinishReset();
		}
	}
}

bool UCrashGameplayAbility_AutoRespawn::CanRespawn_Implementation()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UAttributeSet* LivesAttribute = ASC->GetAttributeSet(ULivesAttributeSet::StaticClass()))
		{
			// TODO: Return true if there is at least one life left.

			return true;
		}
	}

	return false;
}

void UCrashGameplayAbility_AutoRespawn::OnRespawnFailed_Implementation()
{
	// TODO: Make player spectator.
	if (ACrashPlayerController* CrashPC = GetCrashPlayerControllerFromActorInfo())
	{
		CrashPC->ChangeState(NAME_Spectating);
		CrashPC->ResetIgnoreInputFlags();
	}
}

float UCrashGameplayAbility_AutoRespawn::GetRespawnTime_Implementation()
{
	// TODO: Retrieve game mode property.
	return 5.0f;
}

bool UCrashGameplayAbility_AutoRespawn::IsAvatarDeadOrDying() const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (UHealthComponent* HealthComp = UHealthComponent::FindHealthComponent(Avatar))
		{
			return HealthComp->IsDeadOrDying();
		}
	}

	return false;
}
