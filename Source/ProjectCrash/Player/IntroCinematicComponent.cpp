// Copyright Samuel Reitich. All rights reserved.


#include "Player/IntroCinematicComponent.h"

#include "CrashGameplayTags.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"
#include "GameFramework/CrashWorldSettings.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "CrashPlayerState.h"

const FName UIntroCinematicComponent::NAME_ActorFeatureName("IntroCinematic");

void UIntroCinematicComponent::OnRegister()
{
	Super::OnRegister();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();
}

void UIntroCinematicComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for changes to the owning player state's initialization state.
	// BindOnActorInitStateChanged(ACrashPlayerState::NAME_ActorFeatureName, FGameplayTag(), false);

	// Initialize this actor's initialization state, which starts the cinematic.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();

	// TODO: Don't play the cinematic if we joined late and the game has already started.
}

void UIntroCinematicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this actor as an initialization state feature.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UIntroCinematicComponent::OnFirstLoopFinished()
{
	check(IntroCinematicSequenceActor);
	check(IntroCinematicSequenceActor->GetSequencePlayer());
	IntroCinematicSequenceActor->GetSequencePlayer()->OnFinished.Clear();

	// Reset the sequence and start playing it on an infinite loop.
	IntroCinematicSequenceActor->PlaybackSettings.LoopCount.Value = -1;
	IntroCinematicSequenceActor->GetSequencePlayer()->PlayLooping();

	// Progress initialization.
	bFinishedFirstLoop = true;
	CheckDefaultInitialization();
}

bool UIntroCinematicComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	// We can always transition to our initial state when our current state hasn't been initialized yet.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return true;
	}
	// Transition to GameplayReady when the first cinematic loop finishes playing. This actor skips over Initializing.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_GAMEPLAY_READY)
	{
		return bFinishedFirstLoop;
	}

	return false;
}

void UIntroCinematicComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		// Retrieve the intro cinematic from the current world settings.
		if (ACrashWorldSettings* CrashWorldSettings = Cast<ACrashWorldSettings>(GetOwner()->GetWorldSettings()))
		{
			if (CrashWorldSettings->IntroCinematic)
			{
				// Play the intro cinematic for this player, if the level has one.
				// FActorSpawnParameters SpawnParams;
				// SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				// SpawnParams.ObjectFlags |= RF_Transient;
				// SpawnParams.bAllowDuringConstructionScript = true;
				// SpawnParams.bDeferConstruction = true;
				// SpawnParams.Owner = GetOwner();
				//
				// IntroCinematicSequenceActor = GetWorld()->SpawnActor<ALevelSequenceActor>(SpawnParams);
				//
				// FMovieSceneSequencePlaybackSettings Settings = FMovieSceneSequencePlaybackSettings();
				// IntroCinematicSequenceActor->PlaybackSettings = Settings;
				// IntroCinematicSequenceActor->GetSequencePlayer()->SetPlaybackSettings(Settings);
				// IntroCinematicSequenceActor->bReplicatePlayback = false;
				//
				// IntroCinematicSequenceActor->SetSequence(CrashWorldSettings->IntroCinematic);
				// IntroCinematicSequenceActor->InitializePlayer();
				//
				// FTransform DefaultTransform;
				// IntroCinematicSequenceActor->FinishSpawning(DefaultTransform);
				//
				// IntroCinematicSequenceActor->GetSequencePlayer()->Play();

				FMovieSceneSequencePlaybackSettings IntroCinematicSettings = FMovieSceneSequencePlaybackSettings();
				IntroCinematicSettings.bAutoPlay = true;
				IntroCinematicSettings.bHidePlayer = true;
				IntroCinematicSettings.bDisableMovementInput = true;

				ULevelSequencePlayer::CreateLevelSequencePlayer(GetOwner(), CrashWorldSettings->IntroCinematic, IntroCinematicSettings, IntroCinematicSequenceActor);

				// Listen for when the first sequence finishes.
				IntroCinematicSequenceActor->GetSequencePlayer()->OnFinished.AddDynamic(this, &UIntroCinematicComponent::OnFirstLoopFinished);
			}
		}
	}
}

void UIntroCinematicComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// When this component's owning player state transitions to Initializing, stop the cinematic. 
	if (Params.OwningActor == GetPlayerStateChecked<APlayerState>())
	{
		if (Params.FeatureState == STATE_INITIALIZING)
		{
			if (IntroCinematicSequenceActor)
			{
				ULevelSequencePlayer* SequencePlayer = IntroCinematicSequenceActor->GetSequencePlayer();
				if (SequencePlayer && SequencePlayer->IsPlaying())
				{
					SequencePlayer->Stop();

					// Destroy the cinematic actor.
					IntroCinematicSequenceActor->SetLifeSpan(0.1f);
					IntroCinematicSequenceActor = nullptr;
				}
			}
		}
	}
}

void UIntroCinematicComponent::CheckDefaultInitialization()
{
	/* This actor skips the Initializing state and goes straight to GameplayReady when it's ready, so it doesn't block
	 * initialization after it's finished its job. */
	static const TArray<FGameplayTag> StateChain = { STATE_WAITING_FOR_DATA, STATE_GAMEPLAY_READY };
	ContinueInitStateChain(StateChain);
}
