// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_AddWidgets.h"

#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "GameFeaturesSubsystemSettings.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/HUD.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "GameFeatures"

void UGameFeatureAction_AddWidgets::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Reset context data for the given context.
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddWidgets::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Start listening for the extension request that tells this action when to add widgets.
	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			TSoftClassPtr<AActor> HUDClass = AHUD::StaticClass(); // TODO: Change to ACrashHUD

			// Create a delegate for when we should add widgets.
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddWidgetsDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, ChangeContext);

			// Bind the delegate to the HUD to add this action's widgets when it's ready.
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(HUDClass, AddWidgetsDelegate);

			// Cache the delegate for later.
			ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddWidgets::Reset(FPerContextData& ActiveData)
{
	// Clear extension event delegates.
	ActiveData.ExtensionRequestHandles.Empty();

	// Unregister each widget from its extension point.
	for (auto& ExtensionData : ActiveData.ActiveExtensions)
	{
		for (FUIExtensionHandle& ExtensionWidget : ExtensionData.Value.WidgetsAdded)
		{
			ExtensionWidget.Unregister();
		}
	}
}

void UGameFeatureAction_AddWidgets::HandleActorExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Add widgets when requested.
	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFrameworkComponentManager::NAME_GameActorReady))
	{
		AddWidgets(Actor, ActiveData);
	}
	// Remove widgets when requested.
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveWidgets(Actor, ActiveData);
	}
}

void UGameFeatureAction_AddWidgets::AddWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	AHUD* HUD = CastChecked<AHUD>(Actor);

	if (!HUD->GetOwningPlayerController())
	{
		return;
	}

	// Add this action's layouts and widgets to the given HUD actor's owning player.
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(HUD->GetOwningPlayerController()->Player))
	{
		FExtendedActorData& ActorData = ActiveData.ActiveExtensions.FindOrAdd(HUD);

		// Register this action's layouts.
		for (const FGameFeatureHUDLayoutEntry& LayoutEntry : LayoutList)
		{
			if (TSubclassOf<UCommonActivatableWidget> LayoutClass = LayoutEntry.LayoutClass.Get())
			{
				ActorData.LayoutsAdded.Add(UCommonUIExtensions::PushContentToLayer_ForPlayer(LocalPlayer, LayoutEntry.TargetLayer, LayoutClass));
			}
		}

		// Insert this action's widgets into matching widget slots.
		UUIExtensionSubsystem* ExtensionSubsystem = HUD->GetWorld()->GetSubsystem<UUIExtensionSubsystem>();
		for (const FGameFeatureWidgetEntry& WidgetEntry : WidgetList)
		{
			ActorData.WidgetsAdded.Add(ExtensionSubsystem->RegisterExtensionAsWidgetForContext(WidgetEntry.TargetSlot, LocalPlayer, WidgetEntry.WidgetClass.Get(), -1));
		}
	}
}

void UGameFeatureAction_AddWidgets::RemoveWidgets(AActor* Actor, FPerContextData& ActiveData)
{
	AHUD* HUD = CastChecked<AHUD>(Actor);

	// Remove this action's layouts and widgets.
	if (FExtendedActorData* ActorData = ActiveData.ActiveExtensions.Find(HUD))
	{
		// Remove added layouts.
		for (TWeakObjectPtr<UCommonActivatableWidget>& AddedLayout : ActorData->LayoutsAdded)
		{
			if (AddedLayout.IsValid())
			{
				AddedLayout->DeactivateWidget();
			}
		}

		// Remove added widgets.
		for (FUIExtensionHandle& WidgetHandle : ActorData->WidgetsAdded)
		{
			WidgetHandle.Unregister();
		}

		// Clear the actor extension.
		ActiveData.ActiveExtensions.Remove(HUD);
	}
}

#if WITH_EDITORONLY_DATA
void UGameFeatureAction_AddWidgets::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	// Add this action's layouts to the client asset bundle.
	for (const FGameFeatureHUDLayoutEntry& Entry : LayoutList)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.LayoutClass.ToSoftObjectPath().GetAssetPath());
	}

	// Add this action's widgets to the client asset bundle.
	for (const FGameFeatureWidgetEntry& Entry : WidgetList)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, Entry.WidgetClass.ToSoftObjectPath().GetAssetPath());
	}
}
#endif // WITH_EDITORONLY_DATA

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddWidgets::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	// Validate HUD layouts.
	{
		int32 EntryIndex = 0;
		for (const FGameFeatureHUDLayoutEntry& Entry : LayoutList)
		{
			// Validate the HUD layout widget class.
			if (Entry.LayoutClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutEntryHasNullClass", "Null layout widget class at index [{0}] of \"Layouts to Add\" in \"Add Widgets\" action."), FText::AsNumber(EntryIndex)));
			}

			// Validate the HUD layout's target layer.
			if (!Entry.TargetLayer.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("LayoutEntryHasInvalidLayer", "Invalid target layer specified at index [{0}] of \"Layouts to Add\" in \"Add Widgets\" action."), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	// Validate widgets.
	{
		int32 EntryIndex = 0;
		for (const FGameFeatureWidgetEntry& Entry : WidgetList)
		{
			// Validate the widget class.
			if (Entry.WidgetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("WidgetEntryHasNullClass", "Null widget class at index [{0}] of \"Widgets to Add\" in \"Add Widgets\" action."), FText::AsNumber(EntryIndex)));
			}

			// Validate the widget's target slot.
			if (!Entry.TargetSlot.IsValid())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("WidgetEntryHasInvalidSlot", "Invalid target slot specified at index [{0}] of \"Widgets to Add\" in \"Add Widgets\" action."), FText::AsNumber(EntryIndex)));
			}

			++EntryIndex;
		}
	}

	return Result;
}
#endif // WITH_EDITOR