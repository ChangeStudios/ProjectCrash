// Copyright Samuel Reitich. All rights reserved.


#include "Player/CrashPlayerController.h"

#include "CrashPlayerState.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Camera/CrashPlayerCameraManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/SpectatorPawn.h"

ACrashPlayerController::ACrashPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ACrashPlayerCameraManager::StaticClass();
}

void ACrashPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	// Process ability input.
	if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent())
	{
		CrashASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ACrashPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
	OnPlayerStateChanged();
}

void ACrashPlayerController::CleanupPlayerState()
{
	Super::CleanupPlayerState();
	OnPlayerStateChanged();
}

void ACrashPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnPlayerStateChanged();
}

void ACrashPlayerController::OnPlayerStateChanged()
{
	// Unbind from the previous player state.
	FGenericTeamId OldTeamId = FGenericTeamId::NoTeam;
	if (LastSeenPlayerState != nullptr)
	{
		if (ICrashTeamAgentInterface* OldPlayerStateAsTeam = Cast<ICrashTeamAgentInterface>(LastSeenPlayerState))
		{
			OldTeamId = OldPlayerStateAsTeam->GetGenericTeamId();
			OldPlayerStateAsTeam->GetTeamIdChangedDelegateChecked().RemoveAll(this);
		}
	}

	// Bind to the new player state.
	FGenericTeamId NewTeamId = FGenericTeamId::NoTeam;
	if (PlayerState != nullptr)
	{
		if (ICrashTeamAgentInterface* PlayerStateAsTeam = Cast<ICrashTeamAgentInterface>(PlayerState))
		{
			if (ICrashTeamAgentInterface* NewPlayerStateAsTeam = Cast<ICrashTeamAgentInterface>(PlayerState))
			{
				NewTeamId =	NewPlayerStateAsTeam->GetGenericTeamId();
				NewPlayerStateAsTeam->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnPlayerStateChangedTeam);
			}
		}
	}

	// Broadcast the team change if the new player state is on a different team than the old one.
	BroadcastIfTeamChanged(this, OldTeamId, NewTeamId);

	// Cache the new player state as the one to which this controller's team is currently bound.
	LastSeenPlayerState = PlayerState;

	// Broadcast the change.
	PlayerStateChangedDelegate.Broadcast(PlayerState);
}

void ACrashPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	// This controller's team cannot be set directly. Its team is driven by the player state.
	UE_LOG(LogTeams, Error, TEXT("Attempted to set team on player controller [%s]. Teams cannot be set directly on the player controller: player controllers' teams are driven by the player state."), *GetPathNameSafe(this));
}

FGenericTeamId ACrashPlayerController::GetGenericTeamId() const
{
	// The player controller's team is driven by the player state.
	if (const ICrashTeamAgentInterface* PlayerStateAsTeam = Cast<ICrashTeamAgentInterface>(PlayerState))
	{
		return PlayerStateAsTeam->GetGenericTeamId();
	}

	// If we don't have a player state yet, we don't have a team.
	return FGenericTeamId::NoTeam;
}

void ACrashPlayerController::OnPlayerStateChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	// Broadcast this controller's "team changed" delegate when its player state changes teams.
	BroadcastIfTeamChanged(this, IntegerToGenericTeamId(OldTeam), IntegerToGenericTeamId(NewTeam));
}

ASpectatorPawn* ACrashPlayerController::SpawnSpectatorPawn()
{
	// TODO: Spawn a specator pawn at a spectator player start.
	ASpectatorPawn* SpawnedPawn = Super::SpawnSpectatorPawn();

	if (SpawnedPawn)
	{
		SpawnedPawn->SetActorLocation({0.0f, 0.0f, 1000.0f});
	}

	return SpawnedPawn;
}

void ACrashPlayerController::SetSpectating(bool bSpectator)
{
	// Make the player a spectator.
	if (bSpectator)
	{
		ChangeState(NAME_Spectating);

		if (ACrashPlayerState* CrashPS = GetCrashPlayerState())
		{
			CrashPS->SetPlayerConnectionType(EPlayerConnectionType::Spectator);
			CrashPS->SetIsSpectator(true);
		}
	}
	// Make the player a player.
	else
	{
		ChangeState(NAME_Playing);

		if (ACrashPlayerState* CrashPS = GetCrashPlayerState())
		{
			CrashPS->SetPlayerConnectionType(EPlayerConnectionType::Player);
		}
	}

	ResetIgnoreInputFlags();
}

ACrashPlayerState* ACrashPlayerController::GetCrashPlayerState() const
{
	return CastChecked<ACrashPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UCrashAbilitySystemComponent* ACrashPlayerController::GetCrashAbilitySystemComponent() const
{
	const ACrashPlayerState* CrashPS = GetCrashPlayerState();
	return (CrashPS ? CrashPS->GetCrashAbilitySystemComponent() : nullptr);
}
