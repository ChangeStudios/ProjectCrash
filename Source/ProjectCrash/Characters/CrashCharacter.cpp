// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacter.h"

#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"
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


	// First-person mesh.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonCharacterMesh"));
	check(FirstPersonMesh);
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->bOnlyOwnerSee = true;
	FirstPersonMesh->SetupAttachment(CapsuleComp);
	FirstPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	FirstPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));


	// Third-person mesh.
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonCharacterMesh"));
	check(ThirdPersonMesh);
	ThirdPersonMesh->bReceivesDecals = false;
	ThirdPersonMesh->bCastHiddenShadow = true;
	ThirdPersonMesh->bOwnerNoSee = true;
	ThirdPersonMesh->SetupAttachment(CapsuleComp);
	ThirdPersonMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	ThirdPersonMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));


	// TODO: Camera.
	BaseEyeHeight = 166.5f;


	// Input.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}
