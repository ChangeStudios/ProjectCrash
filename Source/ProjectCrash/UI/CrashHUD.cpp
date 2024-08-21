// Copyright Samuel Reitich. All rights reserved.


#include "UI/CrashHUD.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/GameFrameworkComponentManager.h"

ACrashHUD::ACrashHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ACrashHUD::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	// Register with the game framework component manager.
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ACrashHUD::BeginPlay()
{
	/* Send an extension event telling the "AddWidgets" game feature action that the HUD is ready to add the action's
	 * layouts and widgets. */ 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);

	Super::BeginPlay();
}

void ACrashHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from the game framework component manager.
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);

	Super::EndPlay(EndPlayReason);
}

void ACrashHUD::GetDebugActorList(TArray<AActor*>& InOutList)
{
	UWorld* World = GetWorld();
	Super::GetDebugActorList(InOutList);

	// Add all actors with an ability system component.
	for (TObjectIterator<UAbilitySystemComponent> It; It; ++It)
	{
		if (UAbilitySystemComponent* ASC = *It)
		{
			// Don't add CDOs.
			if (!ASC->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				AActor* Avatar = ASC->GetAvatarActor();
				AActor* OwnerActor = ASC->GetOwnerActor();

				// Try to add the ASC's avatar.
				if (Avatar && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Avatar))
				{
					AddActorToDebugList(Avatar, InOutList, World);
				}
				// Add the ASC's owner if they do not have an avatar.
				else if (OwnerActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor))
				{
					AddActorToDebugList(OwnerActor, InOutList, World);
				}
			}
		}
	}
}
