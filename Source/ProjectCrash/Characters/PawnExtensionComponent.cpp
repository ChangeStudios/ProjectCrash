// Copyright Samuel Reitich. All rights reserved.


#include "Characters/PawnExtensionComponent.h"

#include "Characters/Data/PawnData.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

const FName UPawnExtensionComponent::NAME_ActorFeatureName("PawnExtension");

UPawnExtensionComponent::UPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	PawnData = nullptr;
	AbilitySystemComponent = nullptr;
}

void UPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure this component is on a pawn.
	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("Found pawn extension component on [%s]. The pawn extension component can only be added to pawn actors."), *GetNameSafe(GetOwner()));

	// Ensure there is at most ONE instance of this component on any pawn.
	TArray<UActorComponent*> OutPawnExtensionComponents;
	Pawn->GetComponents(ThisClass::StaticClass(), OutPawnExtensionComponents);
	ensureAlwaysMsgf((OutPawnExtensionComponents.Num() == 1), TEXT("Pawn [%s] contains more than one pawn extension component. Only one pawn extension component should exist on any pawn."), *GetNameSafe(GetOwner()));

	// Register this component as a feature with the initialization state framework.
	RegisterInitStateFeature();
}

void UPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	/* Start listening for initialization state changes on this component's owning actor. I.e. changes to its other
	 * components. */
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Initialize this component's initialization state.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Uninitialize the owning pawn from its ASC, if it has one.
	UninitializeAbilitySystem();

	// Unregister this component from the initialization state framework.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UPawnExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	// Transition to the initial state once we have a valid pawn, controller, and pawn data.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		// Check for a valid owning pawn.
		if (!Pawn)
		{
			return false;
		}

		// Check that the possessing controller is valid, if it should be (server or local controller).
		if ((Pawn->HasAuthority()) || (Pawn->IsLocallyControlled()))
		{
			if (!GetController<AController>())
			{
				return false;
			}
		}
	
		// Check for valid pawn data.
		if (!PawnData)
		{
			return false;
		}

		return true;
	}
	// Transition to initializing when all of our features have reached Initializing.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, STATE_INITIALIZING);
	}
	// Transition to GameplayReady once all of our features are GameplayReady.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, STATE_GAMEPLAY_READY);
	}

	return false;
}

void UPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (i.e. another actor component), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UPawnExtensionComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on.
	CheckDefaultInitializationForImplementers();
	ContinueInitStateChain(CrashGameplayTags::StateChain);
}

void UPawnExtensionComponent::HandleControllerChanged()
{
	if (AbilitySystemComponent.Get() && (AbilitySystemComponent->GetAvatarActor() == GetPawnChecked<APawn>()))
	{
		// Ensure the ASC's actor info is still accurate.
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());

		// If the ASC's owner has been reset, uninitialize this pawn from the ASC, as it's no longer usable.
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		// If the ASC still has the correct owner, refresh its actor info the update its new owning controller.
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}

	// Attempt to progress initialization when the controller is updated.
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::HandlePlayerStateReplicated()
{
	// Attempt to progress initialization when the player state is replicated.
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::HandleInputComponentSetUp()
{
	// Attempt to progress initialization when the input component is set up.
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::SetPawnData(const UPawnData* InPawnData)
{
	check(InPawnData);
	APawn* Pawn = GetPawnChecked<APawn>();

	// Only update pawn data on the server.
	if (!Pawn->HasAuthority())
	{
		return;
	}

	// Players cannot change pawn data during the game.
	if (PawnData)
	{
		UE_LOG(LogCrash, Error, TEXT("Tried to set pawn data [%s] on pawn [%s]. This pawn already has valid pawn data: [%s]."),
			*GetNameSafe(InPawnData),
			*GetNameSafe(Pawn),
			*GetNameSafe(PawnData));

		/* @Note: If we wanted a game mode where players can switch pawns during a game (e.g. Overwatch), we would have
		 * to remove the current pawn data's added ability sets and input configuration. */
		return;
	}

	// Update the pawn data.
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;
	Pawn->ForceNetUpdate();

	// Try to progress our initialization, now that we have valid pawn data.
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::OnRep_PawnData()
{
	// Try to progress our initialization once we receive valid pawn data.
	CheckDefaultInitialization();
}

void UPawnExtensionComponent::InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	// Check if the ASC has changed.
	if (AbilitySystemComponent == InASC)
	{
		return;
	}

	// If we already have an ASC, remove it.
	if (AbilitySystemComponent.Get())
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogCrash, Verbose, TEXT("Setting up ASC [%s] on pawn [%s]. ASC Owner: [%s]. Current Avatar: [%s]."), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	// Make sure the given ASC does not already have a pawn avatar.
	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		/** If the given ASC already has a pawn acting as it's avatar, we need to remove it. This can happen on clients
		 * when lagging: the new pawn is spawned and possessed before the old one is removed. */
		ensure(!ExistingAvatar->HasAuthority());

		if (UPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	// Initialize this component's owning pawn as the avatar of the ASC.
	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);
	OnAbilitySystemInitialized.Broadcast();
}

void UPawnExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent.Get())
	{
		return;
	}

	// Uninitialize this pawn from the ASC if it's the current avatar.
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		// Add any other abilities to keep past avatar destruction here.
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(CrashGameplayTags::TAG_Ability_Behavior_PersistsThroughAvatarDestruction);

		// Cancel ongoing abilities.
		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);

		// Cancel gameplay cues.
		AbilitySystemComponent->RemoveAllGameplayCues();

		// Clear the ASC's avatar if it still has a valid owner.
		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		// If the ASC doesn't have an owner anymore, reset all of its current actor information.
		else
		{
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	// Clear the cached ASC.
	AbilitySystemComponent = nullptr;
}

void UPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// Register the given delegate to this component's owning pawn initializes with an ASC.
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	// If this pawn is already initialized with an ASC, invoke the delegate immediately.
	if (AbilitySystemComponent.Get())
	{
		Delegate.Execute();
	}
}

void UPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// Register the given delegate to this component's owning pawn uninitializes itself from an ASC.
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}
