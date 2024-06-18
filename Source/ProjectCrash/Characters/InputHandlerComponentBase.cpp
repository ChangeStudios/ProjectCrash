// Copyright Samuel Reitich. All rights reserved.


#include "Characters/InputHandlerComponentBase.h"

#include "CrashGameplayTags.h"
#include "PawnExtensionComponent.h"
#include "GameFramework/CrashLogging.h"
#include "Misc/UObjectToken.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"

const FName UInputHandlerComponentBase::NAME_ActorFeatureName("InputHandler");
const FName UInputHandlerComponentBase::NAME_BindInputsNow("BindInputsNow");

void UInputHandlerComponentBase::OnRegister()
{
	Super::OnRegister();

	// Make sure this component is only added to pawns.
	if (!GetPawn<APawn>())
	{
		UE_LOG(LogCrash, Error, TEXT("An input handler component was added to [%s], which is not a pawn. This component can only be used on pawns."), *GetNameSafe(GetOwner()))
#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("InputHandlerComponent", "NotOnPawnError", "was added to a class that is not a pawn. This component can only be used on pawns. This will cause a crash if you PIE.");
			static const FName InputHandlerMessageLogName = TEXT("InputHandlerComponent");

			FMessageLog(InputHandlerMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(InputHandlerMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register this component as a feature with the initialization state framework.
		RegisterInitStateFeature();
	}
}

void UInputHandlerComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for initialization state changes on the pawn extension component.
	BindOnActorInitStateChanged(UPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Initialize this component's initialization state.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();
}

void UInputHandlerComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this component from the initialization state framework.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UInputHandlerComponentBase::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	// Transition to the initial state if we have a valid pawn.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return (Pawn != nullptr);
	}
	// Transition to Initializing when we have a valid player state, controller, and input component.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Make sure we have a valid player state.
		if (!GetPlayerState<ACrashPlayerState>())
		{
			return false;
		}

		// For authority or autonomous, wait for a controller with ownership of this pawn's player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bHasControllerPairedWithPS =
				(Controller != nullptr) &&
				(Controller->PlayerState != nullptr) &&
				(Controller->PlayerState->GetOwner() == Controller);

			if (!bHasControllerPairedWithPS)
			{
				return false;
			}
		}

		// Make sure locally controlled players have an input component.
		if ((Pawn->IsLocallyControlled()) && (!Pawn->IsBotControlled()))
		{
			ACrashPlayerController* CrashPC = GetController<ACrashPlayerController>();

			if (!Pawn->InputComponent || !CrashPC || !CrashPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	// Transition to GameplayReady when our input is ready.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		return bReadyToBindInputs;
	}

	return false;
}

void UInputHandlerComponentBase::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// Initialize player input when transitioning to Initializing.
	if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Make sure we have a valid pawn and player state.
		APawn* Pawn = GetPawn<APawn>();
		ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>();
		if (!ensure(Pawn && CrashPS))
		{
			return;
		}

		// Retrieve the pawn data for this component's owning pawn.
		const UPawnData* PawnData = nullptr;
		if (UPawnExtensionComponent* PawnExtensionComponent = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtensionComponent->GetPawnData<UPawnData>();
		}
	}
}

void UInputHandlerComponentBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	IGameFrameworkInitStateInterface::OnActorInitStateChanged(Params);
}

void UInputHandlerComponentBase::CheckDefaultInitialization()
{
	IGameFrameworkInitStateInterface::CheckDefaultInitialization();
}

void UInputHandlerComponentBase::AddAdditionalInputActions(const UCrashInputActionMapping* InputActionMapping)
{
}

void UInputHandlerComponentBase::RemoveAdditionalInputActions(const UCrashInputActionMapping* InputActionMapping)
{
}