// Copyright Samuel Reitich 2024.


#include "ChallengerBase.h"

#include "AbilitySystemLog.h"
#include "ChallengerData.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Abilities/Generic/GA_Death.h"
#include "AbilitySystem/Components/AbilitySystemExtensionComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GlobalGameData.h"
#include "GameFramework/GameStates/CrashGameState.h"
#include "Input/CrashInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/PlayerControllers/CrashPlayerController.h"
#include "Player/PlayerStates/CrashPlayerState.h"

AChallengerBase::AChallengerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		// Do not create the default mesh component. It needs to be nested to the player camera.
		.DoNotCreateDefaultSubobject(MeshComponentName))
{
	// Networking.
	bReplicates = true;
	AActor::SetReplicateMovement(true);


	// Collision component.
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);


	// Camera.
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 60.f)); // Default position.
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->SetFieldOfView(90.0f);
	FirstPersonCameraComponent->bConstrainAspectRatio = true;


	// First-person mesh.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->bCastDynamicShadow = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -165.0f)); // Default position.
	FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Default rotation.


	// Third-person mesh.
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetOwnerNoSee(true);
	ThirdPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -150.0f)); // Default position.
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // Default rotation.


	// Equipment.
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));


	// Ability system.
	ASCExtensionComponent = CreateDefaultSubobject<UAbilitySystemExtensionComponent>(TEXT("AbilitySystemExtensionComponent"));
	ASCExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &AChallengerBase::OnAbilitySystemInitialized));
	ASCExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &AChallengerBase::OnAbilitySystemUninitialized));


	// Attributes.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	// Movement.
	JumpMaxCount = 2; // Implement double-jump by default.


	// Input.
	OverrideInputComponentClass = UCrashInputComponent::StaticClass(); // Use the custom input component.
}

void AChallengerBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Update this character's fresnel when a new controller possesses it on clients.
	UpdateTeamFresnel();
}

void AChallengerBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize the ASC on the server.
	ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>();

	if (!ensure(CrashPS))
	{
		UE_LOG(LogAbilitySystem, Error, TEXT("Actor [%s] tried to initialize its ability system component, but it does not have a player state. This actor needs a player state, or must use a different actor as its ASC's owner."), *GetName());
		return;
	}

	UCrashAbilitySystemComponent* CrashASC = CrashPS->GetCrashAbilitySystemComponent();

	// Initialize the possessing player's ASC with this pawn as the new avatar.
	if (ASCExtensionComponent)
	{
		ASCExtensionComponent->InitializeAbilitySystem(CrashASC, CrashPS);

		// Update the ASC's actor information.
		ASCExtensionComponent->HandleControllerChanged();
	}

	// Update this character's fresnel when a new controller possesses it on the server.
	UpdateTeamFresnel();
}

void AChallengerBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// If this character gets a new player state, initialize its ASC on the client.
	ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>();

	if (!CrashPS)
	{
		return;
	}

	UCrashAbilitySystemComponent* CrashASC = CrashPS->GetCrashAbilitySystemComponent();

	/* Initialize the possessing player's ASC with this pawn as the new avatar. If the ASC was already initialized for
	 * another player, it will be uninitialized first. */
	if (ASCExtensionComponent)
	{
		ASCExtensionComponent->InitializeAbilitySystem(CrashASC, CrashPS);

		// Update the ASC's actor information.
		ASCExtensionComponent->HandleControllerChanged();
	}

	// Update this character's fresnel when a new PS possesses it.
	UpdateTeamFresnel();
}

void AChallengerBase::InitializeGameplayTags()
{
	// Clear any unwanted tags that may be leftover from the ASC's previous pawn.
	if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent())
	{
		FGameplayTagContainer OutTags;
		CrashASC->GetOwnedGameplayTags(OutTags);
		for (FGameplayTag Tag : OutTags.GetGameplayTagArray())
		{
			// Clear any transient movement tags.
			if (Tag.MatchesTag(CrashGameplayTags::TAG_State_Movement))
			{
				CrashASC->SetLooseGameplayTagCount(Tag, 0);
			}
		}
	}
}

void AChallengerBase::UninitAndDestroy()
{
	// Set a timer to safely destroy this actor on the server.
	if (GetLocalRole() == ROLE_Authority)
	{
		// Unregister the equipment component early so it has time to destroy the equipment actors.
		EquipmentComponent->UnregisterComponent();

		DetachFromControllerPendingDestroy();
		SetLifeSpan(0.1f);
	}

	/* Uninitialize this character from its ASC if it's still the avatar. Otherwise, the ASC's new avatar already did
	 * this. This is done here instead of on unpossession because we don't want to remove the ASC until AFTER actors
	 * finish dying, which happens after they are unpossessed. */
	if (UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent())
	{
		if (CrashASC->GetAvatarActor() == this)
		{
			ASCExtensionComponent->UninitializeAbilitySystem();
		}
	}

	// Hide this actor until it's fully destroyed.
	SetActorHiddenInGame(true);
}

void AChallengerBase::HandleDeathStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	/* When the Dying tag is removed, this character's death has finished. We don't need to do anything when the Dying
	 * tag is added because the ASC's DeathEventDelegate will handle starting the death. */
	if (NewCount == 0)
	{
		OnDeathFinished();
	}
}

void AChallengerBase::OnDeathStarted(const FDeathData& DeathData)
{
	// Hide the first-person mesh.
	FirstPersonMesh->SetVisibility(false, true);

	// Reveal third-person mesh to everyone.
	ThirdPersonMesh->SetOwnerNoSee(false);

	// Yeet (technical term) the third-person mesh away depending on the amount of damage that killed this character.
	if (HasAuthority() && DeathData.KillingDamageCauser)
	{
		const FVector SourceLocation = DeathData.KillingDamageCauser->GetActorLocation();
		const FRotator DirectionRot = UKismetMathLibrary::FindLookAtRotation(SourceLocation, GetActorLocation());
		const FVector DirectionVec = DirectionRot.Quaternion().GetForwardVector();
		const float LaunchMultiplier = 200.0f;
		const FVector RagdollVector = ClampVector((DirectionVec * DeathData.DamageMagnitude * LaunchMultiplier), FVector(-100.0f), FVector(100.0f));
		RagdollCharacter(FVector(DirectionVec * DeathData.DamageMagnitude * LaunchMultiplier));
	}
}

void AChallengerBase::OnDeathFinished()
{
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]
	{
		UninitAndDestroy();
	}));
}

void AChallengerBase::RagdollCharacter_Implementation(FVector Direction)
{
	// Ragdoll third-person mesh.
	ThirdPersonMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	ThirdPersonMesh->SetSimulatePhysics(true);
	ThirdPersonMesh->WakeAllRigidBodies();

	// Launch the ragdoll in the given direction.
	ThirdPersonMesh->SetAllPhysicsLinearVelocity(ClampVector(Direction, FVector(-1500.0f), FVector(1500.0f)));
}

void AChallengerBase::UpdateTeamFresnel()
{
	// Global game data and gamemode data determine which materials to use.
	const UGlobalGameData* GlobalGameData = &UCrashAssetManager::Get().GetGlobalGameData();
	const AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	const ACrashGameState* CrashGS = GS ? Cast<ACrashGameState>(GS) : nullptr;
	const UCrashGameModeData* GMData = CrashGS ? CrashGS->GetGameModeData() : nullptr;

	// Get this character's player state.
	const ACrashPlayerState* CharacterPS = GetPlayerState<ACrashPlayerState>();

	// Get the local player's player state to check attitude with this character.
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	APlayerController* LocalPC = LocalPlayer ? LocalPlayer->PlayerController.Get() : Cast<APlayerController>(GetController());
	ACrashPlayerController* LocalCrashPC = LocalPC ? Cast<ACrashPlayerController>(LocalPC) : nullptr;
	const APlayerState* LocalPS = LocalPC ? LocalPC->GetPlayerState<APlayerState>() : nullptr;


	if (CharacterPS && LocalPS && GlobalGameData && GMData)
	{
		/* If we tried updating the fresnel again and it worked this time, clear the delegate that this function was
		 * bound to. */
		LocalCrashPC->PlayerStateChangedDelegate.RemoveAll(this);

		// Set the third-person mesh's color depending on the player state's team.
		switch (FCrashTeamID::GetAttitude(CharacterPS, LocalPS))
		{
			// For characters on the same team, use the constant "friendly" fresnel.
			case Friendly:
			{
				GetThirdPersonMesh()->SetOverlayMaterial(GlobalGameData->TeamFresnel_Friendly);
				break;
			}
			// For non-neutral characters on other teams, use a hostile fresnel.
			case Hostile:
			{
				// If each team should get a distinct hostile fresnel, use their team ID to select one.
				if (GMData->bUseHostileTeamFresnels)
				{
					if (!(GlobalGameData->HostileTeamFresnels.Num() > CharacterPS->GetTeamID()))
					{
						UE_LOG(LogTemp, Fatal, TEXT("Not enough hostile fresnels in global game data for the number of teams in the game!"));
					}

					GetThirdPersonMesh()->SetOverlayMaterial(GlobalGameData->HostileTeamFresnels[CharacterPS->GetTeamID()]);
				}
				// If every hostile character should get the same fresnel, regardless of team, use the generic fresnel.
				else
				{
					GetThirdPersonMesh()->SetOverlayMaterial(GlobalGameData->TeamFresnel_Hostile);
				}

				break;
			}
			// For neutral characters, use the constant "neutral" fresnel.
			default:
			{
				GetThirdPersonMesh()->SetOverlayMaterial(GlobalGameData->TeamFresnel_Neutral);
				break;
			}
		}
	}
	// The local player state may not be ready when this is called. If it's not, try again when it's set.
	else if (LocalCrashPC)
	{
		LocalCrashPC->PlayerStateChangedDelegate.AddDynamic(this, &AChallengerBase::UpdateTeamFresnel);
	}
}

UAbilitySystemComponent* AChallengerBase::GetAbilitySystemComponent() const
{
	/* The interfaced ASC is where we define where the ASC for this class is. For the base challenger, we can access it
	 * through the ability system extension component. */
	if (ASCExtensionComponent == nullptr)
	{
		return nullptr;
	}

	return ASCExtensionComponent->GetCrashAbilitySystemComponent();
}

UCrashAbilitySystemComponent* AChallengerBase::GetCrashAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? Cast<UCrashAbilitySystemComponent>(ASC) : nullptr;
}

void AChallengerBase::OnAbilitySystemInitialized()
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent();
	check(CrashASC);

	// Clear any tags that may be lingering from the ASC's previous pawn.
	InitializeGameplayTags();

	if (!IsValid(ChallengerData))
	{
		ABILITY_LOG(Error, TEXT("[%s]'s ASC could not be properly initialized: Missing ChallengerData asset reference."), *GetName());
		return;
	}

	// Grant this character's default ability set on the server.
	if (CrashASC->IsOwnerActorAuthoritative())
	{
		ChallengerData->DefaultAbilitySet->GiveToAbilitySystem(CrashASC, &GrantedDefaultAbilitySetHandle, this);
		ABILITY_LOG(Verbose, TEXT("Granted [%s]'s default ability set to [%s] on the server."), *GetName(), *GetNameSafe(CrashASC->GetOwnerActor()));
	}

	// Initialize this character's equipment component and grant them their default equipment set.
	if (EquipmentComponent && ChallengerData->DefaultEquipmentSet)
	{
		EquipmentComponent->InitializeComponent();

		// Equip this character's default equipment set on the server. Equipment is server-authoritative and replicated.
		if (HasAuthority())
		{
			EquipmentComponent->EquipEquipmentSet(ChallengerData->DefaultEquipmentSet);
			EQUIPMENT_LOG(Verbose, TEXT("Equipping default set for [%s]..."), *GetName());
		}
	}
	else
	{
		EQUIPMENT_LOG(Verbose, TEXT("Could not equip default set for [%s]."), *GetName());
	}

	// Initialize this character's attribute sets.
	HealthComponent->InitializeWithAbilitySystem(CrashASC, ChallengerData->HealthAttributeBaseValues);

	// Bind death events.
	CrashASC->RegisterGameplayTagEvent(CrashGameplayTags::TAG_State_Dying, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AChallengerBase::HandleDeathStateChanged);
	CrashASC->DeathEventDelegate.AddDynamic(this, &AChallengerBase::OnDeathStarted);

	// Broadcast that this character's ASC was initialized.
	ASCInitializedDelegate.Broadcast(CrashASC);

	ABILITY_LOG(Verbose, TEXT("Initialized ASC for [%s] on the [%s]."), *GetName(), *FString(HasAuthority() ? "SERVER" : "CLIENT"));
}

void AChallengerBase::OnAbilitySystemUninitialized()
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent();
	check(CrashASC);

	// Unbind death events.
	CrashASC->UnregisterGameplayTagEvent(DeathTagChangedDelegate, CrashGameplayTags::TAG_State_Dying, EGameplayTagEventType::NewOrRemoved);
	CrashASC->DeathEventDelegate.RemoveDynamic(this, &AChallengerBase::OnDeathStarted);

	// Remove the ability set granted by this character.
	if (CrashASC->IsOwnerActorAuthoritative())
	{
		GrantedDefaultAbilitySetHandle.RemoveFromAbilitySystem(CrashASC);
	}

	// Uninitialize this character's attribute sets.
	HealthComponent->UninitializeFromAbilitySystem();
}

void AChallengerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	const APlayerController* PlayerController = GetController<APlayerController>();
	check(PlayerController);

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (!ChallengerData)
	{
		UE_LOG(LogInput, Fatal, TEXT("Fatal error: ChallengerData is not defined for Challenger class \"%s.\""), *StaticClass()->GetName());
	}

	// Cache a reference to this character's input component as a CrashInputComponent.
	CrashInputComponent = Cast<UCrashInputComponent>(PlayerInputComponent);

	// Add each mapping context to the local player with its specified priority.
	for (const FPrioritizedInputMappingContext& PrioritizedContext : ChallengerData->DefaultInputMappings)
	{
		Subsystem->AddMappingContext(PrioritizedContext.MappingContext, PrioritizedContext.Priority);
	}

	/* Bind the native input actions from each default native action mapping to handler functions. */
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, CrashGameplayTags::TAG_InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Mouse);
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, CrashGameplayTags::TAG_InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Stick);
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, CrashGameplayTags::TAG_InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

	/* Bind the ability input actions from each default ability action mapping to the ability handler functions. */
	CrashInputComponent->BindAbilityInputActions(ChallengerData->DefaultActionMapping);

	/** Broadcast that this actor's input component was set up. */
	InputComponentInitializedDelegate.Broadcast();
}

void AChallengerBase::Input_Look_Mouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(Value.Y);
	}
}

void AChallengerBase::Input_Look_Stick(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	// Get how long the stick has been held.
	const UWorld* World = GetWorld();
	check(World);
	const float DeltaTime = World->GetDeltaSeconds();

	/* Scale the input action value based on the look-rate and the time the stick has been held before applying it to
	 * this character's controller. */
	if (Value.X != 0.0f)
	{
		// TODO: Add look-rate multiplier.
		const float PitchRateMultiplier = 100.0f;

		AddControllerYawInput(Value.X * DeltaTime * PitchRateMultiplier);
	}

	if (Value.Y != 0.0f)
	{
		// TODO: Add look-rate multiplier.
		const float YawRateMultiplier = 100.0f;

		AddControllerPitchInput(Value.Y * DeltaTime * YawRateMultiplier);
	}
}

void AChallengerBase::Input_Move(const FInputActionValue& InputActionValue)
{
	if (!IsValid(Controller))
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();
	const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

	if (Value.X != 0.0f)
	{
		// Get the movement direction relative to the world.
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(MovementDirection, Value.X);
	}

	if (Value.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(MovementDirection, Value.Y);
	}
}