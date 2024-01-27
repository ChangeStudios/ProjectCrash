// Copyright Samuel Reitich 2024.


#include "ChallengerBase.h"

#include "AbilitySystemLog.h"
#include "ChallengerData.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "AbilitySystem/CrashNativeGameplayTags.h"
#include "AbilitySystem/Components/AbilitySystemExtensionComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Input/CrashInputActionMapping.h"
#include "Input/CrashInputComponent.h"
#include "Player/CrashPlayerState.h"

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


	// Ability system.
	ASCExtensionComponent = CreateDefaultSubobject<UAbilitySystemExtensionComponent>(TEXT("AbilitySystemExtensionComponent"));
	ASCExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	ASCExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));


	// Attributes.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));


	// Movement.
	JumpMaxCount = 2; // Implement double-jump by default.


	// Input.
	OverrideInputComponentClass = UCrashInputComponent::StaticClass(); // Use the custom input component.
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
}

void AChallengerBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize the ASC on the client.
	ACrashPlayerState* CrashPS = GetPlayerState<ACrashPlayerState>();

	if (!ensure(CrashPS))
	{
		UE_LOG(LogAbilitySystem, Error, TEXT("Actor [%s] tried to uninitialize its ability system component, but it does not have a player state. This actor needs a player state, or must use a different actor as its ASC's owner."), *GetName());
		return;
	}

	UCrashAbilitySystemComponent* CrashASC = CrashPS->GetCrashAbilitySystemComponent();

	// Initialize the possessing player's ASC with this pawn as the new avatar.
	if (ASCExtensionComponent)
	{
		ASCExtensionComponent->InitializeAbilitySystem(CrashASC, CrashPS);
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

	// Initialize this character's attribute sets.
	HealthComponent->InitializeWithAbilitySystem(CrashASC, ChallengerData->HealthAttributeBaseValues);

	ABILITY_LOG(Verbose, TEXT("Initialized ASC for [%s] on the [%s]."), *GetName(), *FString(HasAuthority() ? "SERVER" : "CLIENT"));
}

void AChallengerBase::OnAbilitySystemUninitialized()
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent();
	check(CrashASC);

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
	const FCrashNativeGameplayTags& GameplayTags = FCrashNativeGameplayTags::Get();
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, GameplayTags.TAG_InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Mouse);
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, GameplayTags.TAG_InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_Look_Stick);
	CrashInputComponent->BindNativeInputAction(ChallengerData->DefaultActionMapping, GameplayTags.TAG_InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);

	/* Bind the ability input actions from each default ability action mapping to the ability handler functions. */
	CrashInputComponent->BindAbilityInputActions(ChallengerData->DefaultActionMapping);
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
		AddControllerYawInput(Value.X * DeltaTime);
	}

	if (Value.Y != 0.0f)
	{
		// TODO: Add look-rate multiplier.
		AddControllerPitchInput(Value.Y * DeltaTime);
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
