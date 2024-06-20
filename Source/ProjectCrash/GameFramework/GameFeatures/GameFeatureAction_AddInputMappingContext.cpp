// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_AddInputMappingContext.h"

#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Characters/Data/PawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

const FName UGameFeatureAction_AddInputMappingContext::NAME_AddContextsNow("RegisterContextsNow");

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering();

	// Bind a delegate to register this action's mapping contexts when a new game instance starts.
	RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextsForGameInstance);

	// Register this action's mapping contexts for any game instances that have already started.
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		RegisterInputMappingContextsForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Create or reset this action's context data for the given context.
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.ControllersAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Reset this action's context data for the given context.
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	// Unbind delegates for registering mapping contexts when new game instances start.
	FWorldDelegates::OnStartGameInstance.Remove(RegisterInputContextMappingsForGameInstanceHandle);
	RegisterInputContextMappingsForGameInstanceHandle.Reset();

	// Unregister this action's mapping contexts.
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		UnregisterInputMappingContextsForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void UGameFeatureAction_AddInputMappingContext::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Start listening for the extension request that tells this action when to add its contexts.
	if ((GameInstance != nullptr) && (World != nullptr) && (World->IsGameWorld()))
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			// Create a delegate for when we should add the mapping contexts to the player.
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddActionMappingDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleControllerExtension, ChangeContext);

			/* Bind the delegate to controllers in the component manager framework. These controllers have to add the
			 * extension request when they are ready to bind input. */
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddActionMappingDelegate);

			// Cache the delegate for later.
			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::Reset(FPerContextData& ActiveData)
{
	// Clear extension event delegates.
	ActiveData.ExtensionRequestHandles.Empty();

	// Remove the mapping contexts added by this action from each controller to which it was added.
	while (!ActiveData.ControllersAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ActiveData.ControllersAddedTo.Top();
		if (ControllerPtr.IsValid())
		{
			RemoveMappingContext(ControllerPtr->GetLocalPlayer(), ActiveData);
		}
		else
		{
			ActiveData.ControllersAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance && !GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this))
	{
		/* Bind delegates to register/unregister this action's mapping contexts for local players when they're added to
		 * or removed from the game. */
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::RegisterInputMappingContextsForLocalPlayer);
		GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &ThisClass::UnregisterInputMappingContextsForLocalPlayer);

		// Register this action's mapping contexts for each local player already in the game.
		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			RegisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	// Register this action's input mapping contexts for the local player.
	if (ensure(LocalPlayer))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UEnhancedInputUserSettings* Settings = EnhancedInput->GetUserSettings())
			{
				for (const FPrioritizedInputMappingContext& Entry : InputMappingContexts)
				{
					if (UInputMappingContext* IMC = Entry.MappingContext)
					{
						Settings->RegisterInputMappingContext(IMC);
					}
				}
			}
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextsForGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance)
	{
		// Stop listening for local players being added to or removed from the game.
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
		GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);

		// Unregister the input mapping contexts from all local players in the game.
		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			UnregisterInputMappingContextsForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextsForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	// Unregister this action's input mapping contexts from the local player.
	if (ensure(LocalPlayer))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInput = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UEnhancedInputUserSettings* Settings = EnhancedInput->GetUserSettings())
			{
				for (const FPrioritizedInputMappingContext& Entry : InputMappingContexts)
				{
					if (UInputMappingContext* IMC = Entry.MappingContext)
					{
						Settings->UnregisterInputMappingContext(IMC);
					}
				}
			}
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* AsController = CastChecked<APlayerController>(Actor);
	ULocalPlayer* LP = AsController->GetLocalPlayer();
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Remove the mapping contexts when the controller removes its extension request.
	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveMappingContext(LP, ActiveData);
	}
	// Add the mapping contexts when the controller adds its extension request.
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFeatureAction_AddInputMappingContext::NAME_AddContextsNow))
	{
		AddMappingContextForPlayer(LP, ActiveData);
	}
}

void UGameFeatureAction_AddInputMappingContext::AddMappingContextForPlayer(UPlayer* Player, FPerContextData& ActiveData)
{
	// Add this action's input mapping contexts to the local player.
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FPrioritizedInputMappingContext& Entry : InputMappingContexts)
			{
				if (const UInputMappingContext* IMC = Entry.MappingContext)
				{
					InputSystem->AddMappingContext(IMC, Entry.Priority);
				}
			}
		}
		else
		{
			UE_LOG(LogGameFeatures, Error, TEXT("Tried to add action mapping contexts to local player, but the player is not using the enhanced input system. Mapping contexts cannot be added."));
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::RemoveMappingContext(UPlayer* Player, FPerContextData& ActiveData)
{
	// Remove this action's input mapping contexts from the local player.
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			for (const FPrioritizedInputMappingContext& Entry : InputMappingContexts)
			{
				if (const UInputMappingContext* IMC = Entry.MappingContext)
				{
					InputSystem->RemoveMappingContext(IMC);
				}
			}
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputMappingContext::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 Index = 0;
	for (const FPrioritizedInputMappingContext& Entry : InputMappingContexts)
	{
		if (!Entry.MappingContext)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullMappingContext", "Null mapping context at index {0}."), Index));
		}

		Index++;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE