// Copyright Samuel Reitich. All rights reserved.


#include "Characters/PawnExtensionComponent.h"

#include "CrashGameplayTags.h"
#include "Net/UnrealNetwork.h"

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

	// Transition to the initial state once we have a valid pawn.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return (Pawn != nullptr);
	}
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		return true;
	}
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		return true;
	}
}

void UPawnExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	IGameFrameworkInitStateInterface::HandleChangeInitState(Manager, CurrentState, DesiredState);
}

void UPawnExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
}

void UPawnExtensionComponent::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
}

void UPawnExtensionComponent::SetPawnData(const UPawnData* InPawnData)
{
}

void UPawnExtensionComponent::OnRep_PawnData()
{
}

void UPawnExtensionComponent::InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
}

void UPawnExtensionComponent::UninitializeAbilitySystem()
{
}

void UPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
}

void UPawnExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
}

void UPawnExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
}
