// Copyright Samuel Reitich. All rights reserved.


#include "Characters/PawnCameraManager.h"

#include "Camera/CrashCameraComponent.h"
#include "Camera/CrashCameraModeBase.h"
#include "Characters/Data/PawnData.h"
#include "CrashGameplayTags.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "PawnExtensionComponent.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif

const FName UPawnCameraManager::NAME_ActorFeatureName("PawnCameraManager");

UPawnCameraManager::UPawnCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityCameraMode = nullptr;
}

void UPawnCameraManager::OnRegister()
{
	Super::OnRegister();

	APawn* OwningPawn = GetPawn<APawn>();
	
	// Make sure this component is only added to pawns.
	if (!OwningPawn)
	{
		UE_LOG(LogCrash, Error, TEXT("A pawn camera manager component was added to [%s], which is not a pawn. This component can only be used on pawns."), *GetNameSafe(GetOwner()))
#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("PawnCameraManagerComponent", "NotOnPawnError", "was added to a class that is not a pawn. This component can only be used on pawns. This will cause a crash if you PIE.");
			static const FName PawnCameraManagerMessageLogName = TEXT("PawnCameraManagerComponent");

			FMessageLog(PawnCameraManagerMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(PawnCameraManagerMessageLogName).Open();
		}
#endif
	}
	else if (OwningPawn && !OwningPawn->FindComponentByClass<UCrashCameraComponent>())
	{
		UE_LOG(LogCrash, Error, TEXT("A pawn camera manager component was added to [%s], which does not possess a camera component of type CrashCameraComponent."), *GetNameSafe(GetOwner()))
#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("PawnCameraManagerComponent", "NoCameraComponentError", "was added to a class that does not possess a camera component of type CrashCameraComponent. The camera will not be set up properly.");
			static const FName PawnCameraManagerMessageLogName = TEXT("PawnCameraManagerComponent");

			FMessageLog(PawnCameraManagerMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));

			FMessageLog(PawnCameraManagerMessageLogName).Open();
		}
#endif
	}
	else
	{
		// Register this component as a feature with the initialization state framework.
		RegisterInitStateFeature();
	}
}

void UPawnCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// Start listening for initialization state changes on the pawn extension component.
	BindOnActorInitStateChanged(UPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Initialize this component's initialization state.
	ensure(TryToChangeInitState(STATE_WAITING_FOR_DATA));
	CheckDefaultInitialization();
}

void UPawnCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this component from the initialization state framework.
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

bool UPawnCameraManager::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);
	APawn* Pawn = GetPawn<APawn>();

	// Transition to the initial state if we have a valid pawn with a camera component.
	if (!CurrentState.IsValid() && DesiredState == STATE_WAITING_FOR_DATA)
	{
		return Pawn && Pawn->FindComponentByClass<UCrashCameraComponent>();
	}
	// Transition to Initializing when we have a controller.
	else if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Only check for a controller for authority or autonomous.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			return IsValid(GetController<AController>());
		}

		return true;
	}
	// Transition to GameplayReady when our pawn's camera is bound.
	else if (CurrentState == STATE_INITIALIZING && DesiredState == STATE_GAMEPLAY_READY)
	{
		if (UCrashCameraComponent* CameraComponent = UCrashCameraComponent::FindCameraComponent(Pawn))
		{
			return CameraComponent->DetermineCameraModeDelegate.IsBound();
		}
	}

	return false;
}

void UPawnCameraManager::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// Bind to our camera component's camera mode when transitioning to Initializing.
	if (CurrentState == STATE_WAITING_FOR_DATA && DesiredState == STATE_INITIALIZING)
	{
		// Make sure we have a valid pawn.
		APawn* Pawn = GetPawn<APawn>();
		if (!ensure(Pawn))
		{
			return;
		}

		// Retrieve the pawn data for this component's owning pawn.
		const UPawnData* PawnData = nullptr;
		if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UPawnData>();
		}

		// Bind to the pawn's camera component to drive its camera mode.
		if (PawnData)
		{
			if (UCrashCameraComponent* CameraComponent = UCrashCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);

				return;
			}
		}

		UE_LOG(LogCrash, Error, TEXT("Pawn Camera Manager on pawn [%s] was unable to bind to the pawn's camera component."), *GetNameSafe(Pawn));
	}
}

void UPawnCameraManager::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature has changed init states (i.e. another actor component), check if we should too.
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void UPawnCameraManager::CheckDefaultInitialization()
{
	ContinueInitStateChain({ STATE_WAITING_FOR_DATA, STATE_INITIALIZING, STATE_GAMEPLAY_READY });
}

TSubclassOf<UCrashCameraModeBase> UPawnCameraManager::DetermineCameraMode() const
{
	// If there is an ability-driven camera mode override, use it.
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	// Use the pawn's default camera mode.
	if (UPawnExtensionComponent* PawnExtComp = UPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UPawnData* PawnData = PawnExtComp->GetPawnData<UPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void UPawnCameraManager::SetAbilityCameraMode(TSubclassOf<UCrashCameraModeBase> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	// Set the ability camera mode override.
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UPawnCameraManager::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	// Clear the ability camera mode override, if it was originally set by the given ability.
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}
