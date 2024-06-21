// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularPawn.h"
#include "CrashPawn.generated.h"

class UPawnExtensionComponent;

/**
 * A simple pawn that provides functionality with a pawn extension component. Useful for non-character pawns that still
 * need to handle things like input or cameras. Can be possessed by players or AI.
 */
UCLASS()
class PROJECTCRASH_API ACrashPawn : public AModularPawn
{
	GENERATED_BODY()
	
	// Construction.

public:

	/** Default constructor. */
	ACrashPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());



	// Initialization.

// The following events are routed to the pawn extension component.
public:

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;



	// Pawn extension.

protected:

	/** Coordinates initialization of this actor's ASC (if it has one) and its other components. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crash|Pawn", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPawnExtensionComponent> PawnExtComp;
};
