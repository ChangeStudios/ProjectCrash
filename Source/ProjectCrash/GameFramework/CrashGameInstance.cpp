// Copyright Samuel Reitich 2024.


#include "GameFramework/CrashGameInstance.h"

#include "CrashGameplayTags.h"

#include "Components/GameFrameworkComponentManager.h"

UCrashGameInstance::UCrashGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCrashGameInstance::Init()
{
	Super::Init();

	// Register our init states.
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(CrashGameplayTags::TAG_InitState_WaitingForData, false, FGameplayTag());
		ComponentManager->RegisterInitState(CrashGameplayTags::TAG_InitState_Initializing, false, CrashGameplayTags::TAG_InitState_WaitingForData);
		ComponentManager->RegisterInitState(CrashGameplayTags::TAG_InitState_GameplayReady, false, CrashGameplayTags::TAG_InitState_Initializing);
	}
}

void UCrashGameInstance::HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	Super::HandlerUserInitialized(UserInfo, bSuccess, Error, RequestedPrivilege, OnlineContext);

	// Load the local player's settings when they log in.
	if (bSuccess && ensure(UserInfo))
	{
		// TODO: Tell the local player to load their settings.
	}
}
