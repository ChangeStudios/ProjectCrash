// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacter.h"

#include "CrashGameplayTags.h"
#include "PawnExtensionComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/CrashCharacterMovementComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "Camera/CrashCameraComponent.h"
#include "Camera/CrashCameraModeBase.h"
#include "Components/CapsuleComponent.h"
#include "Equipment/EquipmentComponent.h"
#include "GameFramework/CrashLogging.h"
#include "Net/UnrealNetwork.h"

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
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(FName("CrashPawnCapsule"));


	// Camera.
	BaseEyeHeight = 66.85f; // (Character eye height (~76.5) - base third-person animation pelvis offset (7.5) - third-person mesh offset (2.15))
	CameraComponent = CreateDefaultSubobject<UCrashCameraComponent>(TEXT("CrashCameraComponent"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(CapsuleComp);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
	CameraComponent->SetFieldOfView(100.0f);


	// First-person mesh.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonCharacterMesh"));
	check(FirstPersonMesh);
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetVisibility(false, true);
	FirstPersonMesh->SetupAttachment(CameraComponent);
	FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FirstPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -166.5f)); // - (Character eye height (~76.5) + capsule half-height (90.0))
	FirstPersonMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName); // Disable collision on first-person mesh.


	// Third-person mesh.
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonCharacterMesh"));
	check(ThirdPersonMesh);
	ThirdPersonMesh->bReceivesDecals = false;
	ThirdPersonMesh->SetVisibility(true, true);
	ThirdPersonMesh->SetupAttachment(CapsuleComp);
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -92.15f)); // Unreal leaves ~2.15cm of space between the collision capsule and the ground.
	ThirdPersonMesh->SetCollisionProfileName(FName("CrashPawnMesh"));


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


	// Equipment component.
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
}

void ACrashCharacter::PossessedBy(AController* NewController)
{
	// Cache our previous team.
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::PossessedBy(NewController);

	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();

	// Update our team to be our new controller's team.
	if (ICrashTeamAgentInterface* ControllerAsTeam = Cast<ICrashTeamAgentInterface>(Controller))
	{
		TeamId_Internal = ControllerAsTeam->GetGenericTeamId();
		BroadcastIfTeamChanged(this, OldTeamId, TeamId_Internal);

		// Start listening for changes to our new controller's team.
		ControllerAsTeam->GetTeamIdChangedDelegateChecked().AddDynamic(this, &ThisClass::OnControllerChangedTeam);
	}
}

void ACrashCharacter::UnPossessed()
{
	// Stop listening for changes to our unpossessing controller's team.
	const FGenericTeamId OldTeamId = TeamId_Internal;
	if (ICrashTeamAgentInterface* ControllerAsTeam = Cast<ICrashTeamAgentInterface>(Controller))
	{
		ControllerAsTeam->GetTeamIdChangedDelegateChecked().RemoveAll(this);
	}

	Super::UnPossessed();

	// Notify the pawn extension component that the pawn's controller changed.
    PawnExtComp->HandleControllerChanged();

	/* Determine what team this character should be on until it's possessed again. Default implementation keeps this
	 * character on the same team (e.g. if a player is temporarily unpossessing their pawn). */
	TeamId_Internal = DetermineNewTeamAfterPossessionEnds(OldTeamId);
	BroadcastIfTeamChanged(this, OldTeamId, TeamId_Internal);
}

void ACrashCharacter::NotifyControllerChanged()
{
	// Cache our previous team.
	const FGenericTeamId OldTeamId = GetGenericTeamId();

	Super::NotifyControllerChanged();

	// Update our current team to be our new controller's team.
	if (HasAuthority() && (Controller != nullptr))
	{
		if (ICrashTeamAgentInterface* ControllerAsTeam = Cast<ICrashTeamAgentInterface>(Controller))
		{
			TeamId_Internal = ControllerAsTeam->GetGenericTeamId();
			BroadcastIfTeamChanged(this, OldTeamId, TeamId_Internal);
		}
	}
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

void ACrashCharacter::OnStartCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode)
{
	// When blending out of a first-person camera, immediately switch to third-person.
	if (NewCameraMode && (NewCameraMode->GetCameraTypeTag() != CrashGameplayTags::TAG_CameraType_FirstPerson))
	{
		FirstPersonMesh->SetVisibility(false, true);
		ThirdPersonMesh->SetVisibility(true, true);
	}
}

void ACrashCharacter::OnFinishCameraModeBlendIn(UCrashCameraModeBase* PreviousCameraMode, UCrashCameraModeBase* NewCameraMode)
{
	// When blending into a first-person camera, switch to third-person when fully blended.
	if (NewCameraMode && (NewCameraMode->GetCameraTypeTag() == CrashGameplayTags::TAG_CameraType_FirstPerson))
	{
		FirstPersonMesh->SetVisibility(true, true);
		ThirdPersonMesh->SetVisibility(false, true);
	}
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

	// Register the character movement component with the ASC.
	CastChecked<UCrashCharacterMovementComponent>(GetMovementComponent())->InitializeWithAbilitySystem(GetCrashAbilitySystemComponent());
}

void ACrashCharacter::OnAbilitySystemUninitialized()
{
	// Unregister the health component from the ASC.
	HealthComponent->UninitializeFromAbilitySystem();
}

void ACrashCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	// This character's team cannot be set directly. Its team is driven by its controller.
	UE_LOG(LogTeams, Error, TEXT("Attempted to set team on character [%s]. Teams cannot be set directly on characters: characters' teams are driven by their controller."), *GetPathNameSafe(this));
}

void ACrashCharacter::OnControllerChangedTeam(UObject* TeamAgent, int32 OldTeam, int32 NewTeam)
{
	// Update this character's team with its controller's new team.
	const FGenericTeamId OldTeamId = TeamId_Internal;
	TeamId_Internal = IntegerToGenericTeamId(NewTeam);
	BroadcastIfTeamChanged(this, OldTeamId, TeamId_Internal);
}

void ACrashCharacter::OnRep_TeamId_Internal(FGenericTeamId OldTeamId)
{
	// Broadcast the team change when this character's team is assigned or changes.
	BroadcastIfTeamChanged(this, OldTeamId, TeamId_Internal);
}

void ACrashCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamId_Internal)
}