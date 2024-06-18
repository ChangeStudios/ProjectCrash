// Copyright Samuel Reitich. All rights reserved.


#include "Characters/PawnExtensionComponent.h"

#include "AIController.h"
#include "Characters/Data/PawnData.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Player/CrashPlayerState.h"

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

	// Transition to the initial state if we have a valid pawn.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return (Pawn != nullptr);
	}
	// Transition to Initializing when we have a valid controller, ASC, and pawn data.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Check that the possessing controller is valid, if it should be (server or local controller).
		if ((Pawn->HasAuthority()) || (Pawn->IsLocallyControlled()))
		{
			if (!GetController<AController>())
			{
				return false;
			}
		}

		/* The pawn either needs a valid player state (with an ASC) or a self-contained ASC before it can be
		 * initialized. */
		bool bValidPS = IsValid(Pawn->GetPlayerState<ACrashPlayerState>());
		bool bPawnOwnedASC = IsValid(UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(Pawn));
		if (!bValidPS && !bPawnOwnedASC)
		{
			return false;
		}

		// Check for valid pawn data.
		if (!PawnData)
		{
			return false;
		}

		return true;
	}
	// Transition to GameplayReady once all of our features are GameplayReady.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		return Manager->HaveAllFeaturesReachedInitState(Pawn, STATE_GAMEPLAY_READY, UPawnExtensionComponent::NAME_ActorFeatureName);
	}

	return false;
}

void UPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// Initialize the ASC when this pawn's data is ready.
	if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		APawn* Pawn = GetPawnChecked<APawn>();

		// Try to initialize the ability system component using their owning player state.
		if (ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>())
		{
			InitializeAbilitySystem(CrashPS->GetCrashAbilitySystemComponent(), CrashPS);
		}
		// Try to initialize the ability system component using the owning pawn.
		else if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(Pawn))
		{
			InitializeAbilitySystem(CrashASC, Pawn);
		}
		// The pawn extension component expects its owner to have an ASC.
		else
		{
			UE_LOG(LogCrash, Error, TEXT("Failed to initialize ability system component via pawn extension component on [%s]."), *GetNameSafe(Pawn));
		}
	}
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

	// Pawns cannot change their pawn data.
	if (PawnData)
	{
		UE_LOG(LogCrash, Error, TEXT("Tried to set pawn data [%s] on pawn [%s]. This pawn already has valid pawn data: [%s]."),
			*GetNameSafe(InPawnData),
			*GetNameSafe(Pawn),
			*GetNameSafe(PawnData));

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

	UE_LOG(LogCrash, Verbose, TEXT("Setting up ASC [%s] on pawn [%s]. ASC Owner: [%s]. Existing Avatar: [%s]. (%s)"),
		*GetNameSafe(InASC),
		*GetNameSafe(Pawn),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(ExistingAvatar),
		*GetClientServerContextString(GetOwner()));

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
