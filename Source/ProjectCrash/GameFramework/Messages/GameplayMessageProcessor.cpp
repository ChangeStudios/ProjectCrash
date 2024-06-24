// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayMessageProcessor.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void UGameplayMessageProcessor::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for relevant gameplay messages.
	StartListening();
}

void UGameplayMessageProcessor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Stop listening for relevant gameplay messages.
	StopListening();

	// Unregister this component's listener handles.
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	for (FGameplayMessageListenerHandle& Handle : ListenerHandles)
	{
		MessageSubsystem.UnregisterListener(Handle);
	}
	ListenerHandles.Empty();
}

void UGameplayMessageProcessor::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	ListenerHandles.Add(MoveTemp(Handle));
}

double UGameplayMessageProcessor::GetServerTime() const
{
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		return GameState->GetServerWorldTimeSeconds();
	}
	else
	{
		return 0.0;
	}
}

