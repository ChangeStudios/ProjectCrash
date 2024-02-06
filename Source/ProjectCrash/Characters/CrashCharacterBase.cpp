// Copyright Samuel Reitich 2024.


#include "Characters/CrashCharacterBase.h"

#include "AbilitySystem/Components/CrashCharacterMovementComponent.h"


ACrashCharacterBase::ACrashCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass(CharacterMovementComponentName, UCrashCharacterMovementComponent::StaticClass()))
{
}

void ACrashCharacterBase::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();

	JumpedDelegate.Broadcast();
}
