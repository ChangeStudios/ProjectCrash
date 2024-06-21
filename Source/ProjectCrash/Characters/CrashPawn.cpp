// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashPawn.h"

#include "PawnExtensionComponent.h"

ACrashPawn::ACrashPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Pawn extension component.
	PawnExtComp = CreateDefaultSubobject<UPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
}

void ACrashPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();
}

void ACrashPawn::UnPossessed()
{
	Super::UnPossessed();

	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();
}

void ACrashPawn::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	// Notify the pawn extension component that the pawn's controller changed.
	PawnExtComp->HandleControllerChanged();
}

void ACrashPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// Notify the pawn extension component that the pawn's owning player state changed.
	PawnExtComp->HandlePlayerStateReplicated();
}

void ACrashPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Notify the pawn extension component that the pawn's input component has been been set up.
	PawnExtComp->HandleInputComponentSetUp();
}
