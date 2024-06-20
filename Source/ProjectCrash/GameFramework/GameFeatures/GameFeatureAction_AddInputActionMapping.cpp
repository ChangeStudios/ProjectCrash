// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_AddInputActionMapping.h"

#include "EnhancedInputSubsystems.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Input/InputHandlers/InputHandlerComponentBase.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

const FName UGameFeatureAction_AddInputActionMapping::NAME_BindInputsNow("BindInputsNow");

void UGameFeatureAction_AddInputActionMapping::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Create or reset context data for the given context.
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensure(ActiveData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveData.PawnsAddedTo.IsEmpty()))
	{
		Reset(ActiveData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputActionMapping::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Reset the context data for the given context.
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddInputActionMapping::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Start listening for the extension request that tells this action when to bind input.
	if ((GameInstance != nullptr) && (World != nullptr) && (World->IsGameWorld()))
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			// Create a delegate for when we should add the action mappings.
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddActionMappingDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandlePawnExtension, ChangeContext);

			/* Bind the delegate to pawns in the component manager framework. These pawns have to add the extension
			 * request when they are ready to bind input. */
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(APawn::StaticClass(), AddActionMappingDelegate);

			// Cache the delegate for later.
			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputActionMapping::Reset(FPerContextData& ActiveData)
{
	// Clear extension request delegates.
	ActiveData.ExtensionRequestHandles.Empty();

	// Remove the input mappings added by this action from each pawn to which it was added.
	while (!ActiveData.PawnsAddedTo.IsEmpty())
	{
		TWeakObjectPtr<APawn> PawnPtr = ActiveData.PawnsAddedTo.Top();
		if (PawnPtr.IsValid())
		{
			RemoveInputMapping(PawnPtr.Get(), ActiveData);
		}
		else
		{
			ActiveData.PawnsAddedTo.Pop();
		}
	}
}

void UGameFeatureAction_AddInputActionMapping::HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APawn* AsPawn = CastChecked<APawn>(Actor);
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Remove the action mappings when the pawn removes its extension request.
	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMapping(AsPawn, ActiveData);
	}
	// Add the action mappings when the pawn adds its extension request.
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFeatureAction_AddInputActionMapping::NAME_BindInputsNow))
	{
		AddInputMappingForPlayer(AsPawn, ActiveData);
	}
}

void UGameFeatureAction_AddInputActionMapping::AddInputMappingForPlayer(APawn* Pawn, FPerContextData& ActiveData)
{
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());

	if (ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr)
	{
		// Ensure the player is using the enhanced input system.
		if (ensureAlwaysMsgf(LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(), TEXT("Tried to add action mappings to local player, but the player is not using the enhanced input system. Input mappings cannot be added.")))
		{
			// Use the input handler component to add this action's new action mappings.
			if (UInputHandlerComponentBase* InputHandlerComponent = Pawn->FindComponentByClass<UInputHandlerComponentBase>())
			{
				if (InputHandlerComponent->IsReadyToBindInput())
				{
					for (const TSoftObjectPtr<const UCrashInputActionMapping>& Entry : InputActionMappings)
					{
						if (const UCrashInputActionMapping* ActionMapping = Entry.Get())
						{
							InputHandlerComponent->AddAdditionalInputActions(ActionMapping);
						}
					}
				}
			}
			else
			{
				UE_LOG(LogGameFeatures, Error, TEXT("Tried to add action mappings to pawn [%s] from game feature [%s], but the pawn does not have any input handler component. An input handler component is required for dynamic action mapping bindings."), *GetNameSafe(Pawn), *GetNameSafe(this));
			}

			ActiveData.PawnsAddedTo.AddUnique(Pawn);
		}
	}
}

void UGameFeatureAction_AddInputActionMapping::RemoveInputMapping(APawn* Pawn, FPerContextData& ActiveData)
{
	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());

	if (ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr)
	{
		// Ensure the player is using the enhanced input system.
		if (ensureAlwaysMsgf(LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(), TEXT("Tried to remove action mappings to local player, but the player is not using the enhanced input system. Input mappings cannot be removed.")))
		{
			// Use the input handler component to remove this action's added action mappings.
			if (UInputHandlerComponentBase* InputHandlerComponent = Pawn->FindComponentByClass<UInputHandlerComponentBase>())
			{
				for (const TSoftObjectPtr<const UCrashInputActionMapping>& Entry : InputActionMappings)
				{
					if (const UCrashInputActionMapping* ActionMapping = Entry.Get())
					{
						InputHandlerComponent->RemoveAdditionalInputActions(ActionMapping);
					}
				}
			}
		}
	}

	ActiveData.PawnsAddedTo.Remove(Pawn);
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddInputActionMapping::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 Index = 0;
	for (const TSoftObjectPtr<const UCrashInputActionMapping>& Entry : InputActionMappings)
	{
		if (Entry.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("NullActionMapping", "Null action mapping at index {0}."), Index));
		}
		else if (const UCrashInputActionMapping* ActionMapping = Entry.Get())
		{
			if (ActionMapping->NativeInputActions.Num() > 0)
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("MappingWithNativeActions", "Action mapping {0} defines native actions. Native actions cannot be added by the game mode."), FText::FromString(GetNameSafe(ActionMapping))));
			}
		}

		Index++;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE