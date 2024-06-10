// Copyright Samuel Reitich. All rights reserved.


#include "Player/PlayerStates/CrashPlayerStateBase.h"

#include "CrashGameplayTags.h"
#include "Characters/Data/ChallengerData.h"
#include "Characters/Data/ChallengerSkinData.h"
#include "GameFramework/GameStates/CrashGameStateBase.h"
#include "Net/UnrealNetwork.h"

const FName ACrashPlayerStateBase::NAME_ActorFeatureName("CrashPlayerState");

void ACrashPlayerStateBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register this actor as a feature with the initialization state framework.
	RegisterInitStateFeature();

	// Start listening for initialization state changes to this actor.
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
}

void ACrashPlayerStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the initialization state.
	ensure(TryToChangeInitState(CrashGameplayTags::TAG_InitState_WaitingForData));
	CheckDefaultInitialization();
}

void ACrashPlayerStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from initialization states.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool ACrashPlayerStateBase::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	return true;
}

void ACrashPlayerStateBase::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// When transitioning to WaitingForData, listen for game mode data and load Challenger data and skin data
}

void ACrashPlayerStateBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (e.g. one of our components), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void ACrashPlayerStateBase::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain(CrashGameplayTags::StateChain);
}

const UChallengerData* ACrashPlayerStateBase::GetCurrentChallenger() const
{
	return CurrentChallenger.Get();
}

const UChallengerSkinData* ACrashPlayerStateBase::GetCurrentSkin() const
{
	return CurrentSkin.Get();
}

void ACrashPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrashPlayerStateBase, CurrentChallenger);
	DOREPLIFETIME(ACrashPlayerStateBase, CurrentSkin);
}