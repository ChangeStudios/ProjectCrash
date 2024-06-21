// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacter.h"

#include "PawnExtensionComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "Camera/CrashCameraComponent.h"
#include "Components/CapsuleComponent.h"


ACrashCharacter::ACrashCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		// Do not create the default mesh component. We create our own for first- and third-person.
		.DoNotCreateDefaultSubobject(MeshComponentName)
		// Use this project's default character movement component.
		.SetDefaultSubobjectClass<UCrashCharacterMovementComponent>(CharacterMovementComponentName))
{
	// Disable ticking.
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;


	// Capsule component.
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(35.0f, 90.0f);


	// Camera.
	BaseEyeHeight = 76.5f;
	CameraComponent = CreateDefaultSubobject<UCrashCameraComponent>(TEXT("CrashCameraComponent"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(CapsuleComp);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));


	// First-person mesh.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonCharacterMesh"));
	check(FirstPersonMesh);
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetupAttachment(CameraComponent);
	FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FirstPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -166.5f));


	// Third-person mesh.
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonCharacterMesh"));
	check(ThirdPersonMesh);
	ThirdPersonMesh->bReceivesDecals = false;
	ThirdPersonMesh->bCastHiddenShadow = true;
	ThirdPersonMesh->SetupAttachment(CapsuleComp);
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));


	// Input.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// Pawn extension component.
	PawnExtComp = CreateDefaultSubobject<UPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComp->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComp->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));


	// Health component.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ACrashCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();
}

void ACrashCharacter::UnPossessed()
{
	Super::UnPossessed();

	// Notify the pawn extension component that the pawn's controller changed.
    PawnExtComp->HandleControllerChanged();
}

void ACrashCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();
}

void ACrashCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Notify the pawn extension component that the pawn's owning player state changed.
	PawnExtComp->HandlePlayerStateReplicated();
}

void ACrashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Notify the pawn extension component that the pawn's input component has been been set up.
	PawnExtComp->HandleInputComponentSetUp();
}

UCrashAbilitySystemComponent* ACrashCharacter::GetCrashAbilitySystemComponent() const
{
	// Use the interfaced ASC getter so this function works with subclasses that override it.
	return Cast<UCrashAbilitySystemComponent>(GetAbilitySystemComponent());
}

UAbilitySystemComponent* ACrashCharacter::GetAbilitySystemComponent() const
{
	check(PawnExtComp);

	/* Get this character's ASC via its pawn extension component. This avoids having to know where the ASC is stored
	 * (e.g. player state vs pawn). */
	return PawnExtComp->GetCrashAbilitySystemComponent();
}

void ACrashCharacter::OnAbilitySystemInitialized()
{
	// Register the health component with the ASC.
	HealthComponent->InitializeWithAbilitySystem(GetCrashAbilitySystemComponent());
}

void ACrashCharacter::OnAbilitySystemUninitialized()
{
	// Unregister the health component from the ASC.
	HealthComponent->UninitializeFromAbilitySystem();
}
