// Copyright Samuel Reitich. All rights reserved.


#include "Characters/PawnCameraManager.h"

const FName UPawnCameraManager::NAME_ActorFeatureName("PawnCameraManager");


UPawnCameraManager::UPawnCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPawnCameraManager::OnRegister()
{
	Super::OnRegister();
}

void UPawnCameraManager::BeginPlay()
{
	Super::BeginPlay();
}

void UPawnCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

bool UPawnCameraManager::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	return IGameFrameworkInitStateInterface::CanChangeInitState(Manager, CurrentState, DesiredState);
}

void UPawnCameraManager::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

void UPawnCameraManager::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
}

void UPawnCameraManager::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
}

TSubclassOf<UCrashCameraModeBase> UPawnCameraManager::DetermineCameraMode() const
{
	return nullptr;
}

void UPawnCameraManager::SetAbilityCameraMode(TSubclassOf<UCrashCameraModeBase> CameraMode,
	const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
}

void UPawnCameraManager::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
}
