// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Abilities/CrashGameplayAbility_AutoRespawn.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayAbility_Reset.h"
#include "CrashGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystem/AttributeSets/LivesAttributeSet.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Player/CrashPlayerState.h"
#include "GameFramework/GameModes/CrashGameMode.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/Messages/CrashSimpleDurationMessage.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
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
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
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

	ResetListener.Unregister();

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
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		ResetListener = MessageSystem.RegisterListener(CrashGameplayTags::TAG_Message_Player_Reset, this, &ThisClass::OnResetMessageReceived);

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
				// Broadcast the respawn message with the respawn duration.
				const float RespawnTime = GetRespawnTime();
				if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
				{
					FCrashSimpleDurationMessage RespawnStartMessage;
					RespawnStartMessage.Instigator = GetCrashPlayerStateFromActorInfo();
					RespawnStartMessage.Duration = RespawnTime;

					UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
					MessageSystem.BroadcastMessage(CrashGameplayTags::TAG_Message_Player_Respawn_Started, RespawnStartMessage);
				}

				// Start timer to respawn.
				/* NOTE: I'm not a huge fan of the respawn being predicted. We might change it to be server-only in the
				 * future. To do this, we'd wrap this timer in a server check (CurrentActorInfo->IsNetAuthority) and
				 * only broadcast the RespawnCompleted message from the server. We could also change this ability's
				 * execution policy to server-initiated. */
				bShouldFinishReset = true;
				FTimerHandle TimerHandle;
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UCrashGameplayAbility_AutoRespawn::OnRespawnTimerEnd, RespawnTime, false);
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
	bShouldFinishReset = false;

	if (IsValid(ControllerToReset))
	{
		// Restart the player on the server.
		if (CurrentActorInfo->IsNetAuthority())
		{
			AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
			ACrashGameMode* CrashGM = GM ? Cast<ACrashGameMode>(GM) : nullptr;
			if (ensure(IsValid(CrashGM)))
			{
				CrashGM->RequestPlayerRestartNextTick(ControllerToReset, true);
			}
		}

		// Broadcast a message indicating that the respawn finished successfully.
		OnRespawnCompleted();
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
	bShouldFinishReset = false;

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

void UCrashGameplayAbility_AutoRespawn::OnResetMessageReceived(FGameplayTag Channel, const FCrashPlayerResetMessage& Message)
{
	if (Message.OwningActor == GetOwningActorFromActorInfo())
	{
		// Clean up this ability, interrupting any ongoing death or reset logic.
		if (CurrentActorInfo->IsNetAuthority())
		{
			StopListeningForDeath();
			bShouldFinishReset = false;
			OnRespawnCompleted();
		}
	}
}

void UCrashGameplayAbility_AutoRespawn::OnAvatarEndPlay(AActor* Avatar, EEndPlayReason::Type Reason)
{
	if (Reason == EEndPlayReason::Type::Destroyed)
	{
		if (CurrentActorInfo->IsNetAuthority())
		{
			ensureAlwaysMsgf(true, TEXT("Avatar [%s] destroyed before death without player reset!"), *GetNameSafe(Avatar));

			// Perform an instant reset. This should never happen; the ResetPlayer event should be used instead.
			ControllerToReset = GetControllerFromActorInfo();
			StopListeningForDeath();
			FinishReset();
		}
	}
}

void UCrashGameplayAbility_AutoRespawn::OnRespawnCompleted()
{
	if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		// Broadcast a RespawnCompleted message.
		FCrashVerbMessage RespawnCompletedMessage;
		RespawnCompletedMessage.Verb = CrashGameplayTags::TAG_Message_Player_Respawn_Completed;
		RespawnCompletedMessage.Instigator = GetCrashPlayerStateFromActorInfo();

		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.BroadcastMessage(RespawnCompletedMessage.Verb, RespawnCompletedMessage);

		/* FinishReset isn't always called locally if there's a missed prediction, so we have to replicate the
		 * message to clients to ensure they always receive it. */
		if (CurrentActorInfo->IsNetAuthority())
		{
			if (ACrashGameState* CrashGS = GetWorld()->GetGameState<ACrashGameState>())
			{
				CrashGS->MulticastReliableMessageToClients(RespawnCompletedMessage);
			}
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
	// Make the player a spectator.
	if (ACrashPlayerController* CrashPC = GetCrashPlayerControllerFromActorInfo())
	{
		CrashPC->ChangeState(NAME_Spectating);
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
