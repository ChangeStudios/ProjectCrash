// Copyright Samuel Reitich. All rights reserved.


#include "InputHandlerComponentBase.h"

#include "CrashGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Characters/PawnExtensionComponent.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameFeatures/GameFeatureAction_AddInputActionMapping.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "Input/CrashInputComponent.h"
#include "Player/CrashPlayerController.h"
#include "Player/CrashPlayerState.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

const FName UInputHandlerComponentBase::NAME_ActorFeatureName("InputHandler");

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
		if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UPawnData>();
		}

		// Initialize the owning pawn's input component.
		if (ACrashPlayerController* CrashPC = GetController<ACrashPlayerController>())
		{
			if (Pawn->InputComponent)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

void UInputHandlerComponentBase::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (i.e. another actor component), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UInputHandlerComponentBase::CheckDefaultInitialization()
{
	ContinueInitStateChain({ STATE_WAITING_FOR_DATA, STATE_INITIALIZING, STATE_GAMEPLAY_READY });
}

void UInputHandlerComponentBase::AddAdditionalInputActions(const UCrashInputActionMapping* ActionMapping)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	// Bind the given action mapping's ability actions.
	UCrashInputComponent* CrashIC = Pawn->FindComponentByClass<UCrashInputComponent>();
	if (ensureAlwaysMsgf(CrashIC, TEXT("Pawn [%s] is not using an input component of type CrashInputComponent. Input will not be properly bound."), *GetNameSafe(Pawn)))
	{
		CrashIC->BindAbilityInputActions(ActionMapping);
	}
}

void UInputHandlerComponentBase::RemoveAdditionalInputActions(const UCrashInputActionMapping* ActionMapping)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	// Remove the given action mapping's ability actions.
	UCrashInputComponent* CrashIC = Pawn->FindComponentByClass<UCrashInputComponent>();
	if (ensureAlwaysMsgf(CrashIC, TEXT("Pawn [%s] is not using an input component of type CrashInputComponent. Input will not be properly bound."), *GetNameSafe(Pawn)))
	{
		CrashIC->RemoveAbilityInputActions(ActionMapping);
	}
}

void UInputHandlerComponentBase::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(InputSubsystem);

	// Clear all existing action mappings.
	InputSubsystem->ClearAllMappings();

	// Retrieve the owning pawn's pawn data using the extension component. 
	if (const UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UPawnData* PawnData = PawnExtComp->GetPawnData<UPawnData>())
		{
			if (const UCrashInputActionMapping* ActionMapping = PawnData->DefaultActionMapping)
			{
				// Add the input mapping context to the local player.
				if (UInputMappingContext* IMC = PawnData->DefaultMappingContext.MappingContext.Get())
				{
					// Register the new input mapping with the user's settings.
					if (UEnhancedInputUserSettings* Settings = InputSubsystem->GetUserSettings())
					{
						Settings->RegisterInputMappingContext(IMC);
					}

					// Add the input mapping to the player.
					FModifyContextOptions Options = {};
					Options.bIgnoreAllPressedKeysUntilRelease = false;
					InputSubsystem->AddMappingContext(IMC, PawnData->DefaultMappingContext.Priority, Options);
				}

				// Ensure we're using the correct input component.
				UCrashInputComponent* CrashIC = Cast<UCrashInputComponent>(PlayerInputComponent);
				if (ensureAlwaysMsgf(CrashIC, TEXT("Pawn [%s] is not using an input component of type CrashInputComponent. Input will not be properly bound."), *GetNameSafe(GetOwner())))
				{
					// Bind the mapping's ability actions.
					CrashIC->BindAbilityInputActions(ActionMapping);

					// Bind any input handler functions defined in this component.
					BindInputHandlers(CrashIC, ActionMapping);
				}
			}
		}
	}

	// Prevent input from being bound multiple times.
	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	/* Tell the modular game framework that we are ready to bind input. This is used to add additional game
	 * mode-specific input bindings. */
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), UGameFeatureAction_AddInputActionMapping::NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), UGameFeatureAction_AddInputActionMapping::NAME_BindInputsNow);
}
