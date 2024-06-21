// Copyright Samuel Reitich. All rights reserved.


#include "GameFramework/GameFeatures/GameFeatureAction_AddAbilities.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

const FName UGameFeatureAction_AddAbilities::NAME_AbilitiesReady("AbilitiesReady");

void UGameFeatureAction_AddAbilities::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	// Create or reset context data for the given context.
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);
	if (!ensureAlways(ActiveData.ActiveExtensions.IsEmpty()) ||
		!ensureAlways(ActiveData.ExtensionRequestHandles.IsEmpty()))
	{
		Reset(ActiveData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	// Reset the context data for the given context.
	FPerContextData* ActiveData = ContextData.Find(Context);
	if (ensure(ActiveData))
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext);

	// Start listening for the extension request that tells this action when to grant abilities.
	if ((GameInstance != nullptr) && (World != nullptr) && (World->IsGameWorld()))
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			int32 EntryIndex = 0;

			for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					// Create a delegate for when we should grant the abilities and attribute sets.
					UGameFrameworkComponentManager::FExtensionHandlerDelegate GrantAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(this, &ThisClass::HandleActorExtension, EntryIndex, ChangeContext);

					/* Bind the delegate to actors of the specified class in the component manager framework. These
					 * actors have to add the extension request when they are ready to be granted the abilities and
					 * attribute sets. */
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(Entry.ActorClass, GrantAbilitiesDelegate);

					// Cache the delegate for later.
					ActiveData.ExtensionRequestHandles.Add(ExtensionRequestHandle);

					++EntryIndex;
				}
			}
		}
	}
}

void UGameFeatureAction_AddAbilities::Reset(FPerContextData& ActiveData)
{
	// Clear extension event delegates.
	ActiveData.ExtensionRequestHandles.Empty();

	// Remove all abilities, ability sets, and attribute sets granted by this action.
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto ExtensionIt = ActiveData.ActiveExtensions.CreateIterator();
		RemoveActorAbilities(ExtensionIt->Key, ActiveData);
	}
}

void UGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);

	if (AbilitiesList.IsValidIndex(EntryIndex) && ActiveData)
	{
		const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];

		// Remove the abilities and attribute sets when the actor removes its extension request.
		if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			RemoveActorAbilities(Actor, *ActiveData);
		}
		// Add the abilities and attribute sets when the actor adds its extension request.
		else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UGameFeatureAction_AddAbilities::NAME_AbilitiesReady))
		{
			AddActorAbilities(Actor, Entry, *ActiveData);
		}
	}
}

void UGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
	check(Actor);

	// Only grant abilities and attribute sets on the server.
	if (!Actor->HasAuthority())
	{
		return;
	}

	// Don't grant abilities if the actor already has them.
	if (ActiveData.ActiveExtensions.Find(Actor) != nullptr)
	{
		return;
	}


	if (UAbilitySystemComponent* AbilitySystemComponent = Actor->FindComponentByClass<UAbilitySystemComponent>())
	{
		// Allocate the memory we'll need to store our granted handles.
		FActorExtensions AddedExtensions;
		AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
		AddedExtensions.AttributeSets.Reserve(AbilitiesEntry.GrantedAttributeSets.Num());
		AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

		// Grant abilities.
		for (const TSoftClassPtr<UGameplayAbility>& Ability : AbilitiesEntry.GrantedAbilities)
		{
			if (!Ability.IsNull())
			{
				FGameplayAbilitySpec NewAbilitySpec(Ability.LoadSynchronous());
				FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);

				// Cache the granted ability.
				AddedExtensions.Abilities.Add(AbilityHandle);
			}
		}


		// Grant ability sets. ASC must be of type CrashAbilitySystemComponent to receive ability sets.
		UCrashAbilitySystemComponent* CrashASC = CastChecked<UCrashAbilitySystemComponent>(AbilitySystemComponent);
		for (const TSoftObjectPtr<const UCrashAbilitySet>& AbilitySetPtr : AbilitiesEntry.GrantedAbilitySets)
		{
			if (const UCrashAbilitySet* AbilitySet = AbilitySetPtr.Get())
			{
				// This grants AND caches the ability set.
				AbilitySet->GiveToAbilitySystem(CrashASC, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef());
			}
		}


		// Grant attribute sets.
		for (const FGameFeatureAbilitiesEntry_AttributeSet& AttributeSet : AbilitiesEntry.GrantedAttributeSets)
		{
			if (!AttributeSet.AttributeSetClass.IsNull())
			{
				TSubclassOf<UAttributeSet> SetClass = AttributeSet.AttributeSetClass.LoadSynchronous();
				if (SetClass)
				{
					// Create the new attribute set.
					UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystemComponent->GetOwner(), SetClass);

					// Initialize the attribute set with the initialization data table, if one was given.
					if (!AttributeSet.InitializationData.IsNull())
					{
						UDataTable* InitData = AttributeSet.InitializationData.LoadSynchronous();
						if (InitData)
						{
							NewSet->InitFromMetaDataTable(InitData);
						}
					}

					// Cache the new attribute set.
					AddedExtensions.AttributeSets.Add(NewSet);
					AbilitySystemComponent->AddAttributeSetSubobject(NewSet);
				}
			}
		}


		// Cache the new extension.
		ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
	}
	else
	{
		UE_LOG(LogGameFeatures, Error, TEXT("GameFeatureAction_AddAbilities failed to find an ability system component for [%s]. Abilities and attribute sets will not be granted."), *Actor->GetPathName());
	}
}

void UGameFeatureAction_AddAbilities::RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData)
{
	if (FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			// Remove the granted abilities (allow them to finish).
			for (FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
			{
				AbilitySystemComponent->SetRemoveAbilityOnEnd(AbilityHandle);
			}

			// Remove the granted ability sets.
			UCrashAbilitySystemComponent* CrashASC = CastChecked<UCrashAbilitySystemComponent>(AbilitySystemComponent);
			for (FCrashAbilitySet_GrantedHandles& AbilitySetHandle : ActorExtensions->AbilitySetHandles)
			{
				AbilitySetHandle.RemoveFromAbilitySystem(CrashASC);
			}

			// Remove the granted attribute sets.
			for (UAttributeSet* AttributeSetInstance : ActorExtensions->AttributeSets)
			{
				AbilitySystemComponent->RemoveSpawnedAttribute(AttributeSetInstance);
			}
		}

		// Clear the cached extension.
		ActiveData.ActiveExtensions.Remove(Actor);
	}
}

#if WITH_EDITOR
EDataValidationResult UGameFeatureAction_AddAbilities::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		// Make sure this action actually does something (has something to grant).
		if (Entry.GrantedAbilities.IsEmpty() && Entry.GrantedAttributeSets.IsEmpty() && Entry.GrantedAbilitySets.IsEmpty())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryGrantsNothing", "Index [{0}] in \"Add Abilities/Attributes\" action will do nothing: No granted abilities, attribute sets, or ability sets are defined."), FText::AsNumber(EntryIndex)));
		}

		// Validate the actor class.
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("EntryHasNullActor", "Null actor class at index [{0}] in \"Add Abilities/Attributes\" action."), FText::AsNumber(EntryIndex)));
		}

		// Validate granted abilities.
		int32 AbilityIndex = 0;
		for (const TSoftClassPtr<UGameplayAbility>& Ability : Entry.GrantedAbilities)
		{
			if (Ability.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNullAbility", "Null ability at index [{0}] in \"Add Abilities/Attributes\" action [{1}]'s granted abilities."), FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}

			++AbilityIndex;
		}

		// Validate granted ability sets.
		int32 AbilitySetIndex = 0;
		for (const TSoftObjectPtr<const UCrashAbilitySet>& AbilitySet : Entry.GrantedAbilitySets)
		{
			if (AbilitySet.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNullAbilitySet", "Null ability set at index [{0}] in \"Add Abilities/Attributes\" action [{1}]'s granted ability sets."), FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}

			++AbilitySetIndex;
		}

		// Validate attribute sets.
		int32 AttributeSetIndex = 0;
		for (const FGameFeatureAbilitiesEntry_AttributeSet& AttributeSets : Entry.GrantedAttributeSets)
		{
			if (AttributeSets.AttributeSetClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("EntryHasNullAttributeSet", "Null attribute set at index {0} in \"Add Abilities/Attributes\" action {1}'s granted attribute sets."), FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}

			++AttributeSetIndex;
		}

		++EntryIndex;
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE