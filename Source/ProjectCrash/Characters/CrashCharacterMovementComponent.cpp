// Copyright Samuel Reitich. All rights reserved.


#include "Characters/CrashCharacterMovementComponent.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/AttributeSets/MovementAttributeSet.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

/** The maximum distance with which we'll perform traces for retrieving ground information. */
#define MAX_GROUND_TRACE_DISTANCE 100000.0f

UCrashCharacterMovementComponent::UCrashCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
{
	GravityScale = 1.5;
	MaxAcceleration = 16384.0f;
	// TODO: Disable friction (BrakingFriction = 0, BrakingFrictionMultiplier = 0, bUseSeparateBrakingFriction = false)
	BrakingFriction = 4.0f;
	bUseSeparateBrakingFriction = true;

	GroundFriction = 0.0f;
	MaxWalkSpeed = 600.0f;
	MaxWalkSpeedCrouched = 300.0f;
	MinAnalogWalkSpeed = 50.0f;
	BrakingDecelerationWalking = 8192.0f;

	JumpZVelocity = 1000.0f;
	AirControl = 0.5f;
	AirControlBoostVelocityThreshold = 0.0f;
	FallingLateralFriction = 0.4;
}

void UCrashCharacterMovementComponent::InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC)
{
	check(CharacterOwner);

	// Don't initialize this component if it's already been initialized with an ASC.
	if (AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize character movement component on [%s]. The component has already been initialized with an ability system."), *GetNameSafe(CharacterOwner));
		return;
	}

	// Don't initialize this component without a valid ASC.
	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize character movement component on [%s]. An invalid ability system component was given."), *GetNameSafe(CharacterOwner));
		return;
	}

	// Don't initialize this component if the given ASC does not have a MovementAttributeSet.
	MovementSet = AbilitySystemComponent->GetSet<UMovementAttributeSet>();
	if (!MovementSet)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize character movement component on [%s] with ASC owned by [%s]. The ASC does not have an attribute set of type MovementAttributeSet."), *GetNameSafe(CharacterOwner), *GetNameSafe(AbilitySystemComponent->GetOwnerActor()));
		return;
	}

	// Register to listen for changes to movement attributes.
	MovementSet->MaxWalkSpeedAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnMaxWalkSpeedChanged);
	MovementSet->JumpVelocityAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnJumpVelocityChanged);
	MovementSet->JumpVelocityAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnJumpCountChanged);

	// Initialize movement properties.
	MaxWalkSpeed = MovementSet->GetMaxWalkSpeed();
	JumpZVelocity = MovementSet->GetJumpVelocity();
	CharacterOwner->JumpMaxCount = MovementSet->GetJumpCount();
}

void UCrashCharacterMovementComponent::UninitializeFromAbilitySystem()
{
	// Clear health event delegates.
	if (MovementSet)
	{
		MovementSet->MaxWalkSpeedAttributeChangedDelegate.RemoveAll(this);
		MovementSet->JumpVelocityAttributeChangedDelegate.RemoveAll(this);
		MovementSet->JumpVelocityAttributeChangedDelegate.RemoveAll(this);
	}

	// Clear cached variables.
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UCrashCharacterMovementComponent::OnUnregister()
{
	// Uninitialize this component when it's unregistered, in case UninitializeFromAbilitySystem wasn't called manually.
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UCrashCharacterMovementComponent::OnMaxWalkSpeedChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	// Copy the change into this component.
	if (MovementSet)
	{
		MaxWalkSpeed = MovementSet->GetMaxWalkSpeed();
	}
}

void UCrashCharacterMovementComponent::OnJumpVelocityChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	// Copy the change into this component.
	if (MovementSet)
	{
		JumpZVelocity = MovementSet->GetJumpVelocity();
	}
}

void UCrashCharacterMovementComponent::OnJumpCountChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	// Copy the change into the owning character.
	if (CharacterOwner)
	{
		CharacterOwner->JumpMaxCount = MovementSet->GetJumpCount();
	}
}

void UCrashCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Disable separate braking friction while airborne.
	bUseSeparateBrakingFriction = (MovementMode != MOVE_Falling);
}

float UCrashCharacterMovementComponent::GetGroundDistance() const
{
	// This character is already on the ground. 
	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		return 0.0f;
	}

	// Collect trace parameters.
	const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
	check(CapsuleComp);

	const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
	const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
	const FVector TraceStart = GetActorLocation();
	const FVector TraceEnd = FVector(TraceStart.X, TraceStart.Y, TraceStart.Z - MAX_GROUND_TRACE_DISTANCE - CapsuleHalfHeight);

	FCollisionQueryParams QueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(UCrashCharacterMovementComponent_GetGroundDistance), false, CharacterOwner);
	FCollisionResponseParams ResponseParams;
	InitCollisionParams(QueryParams, ResponseParams);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParams);

	// If the ground was hit, return the distance to it from the bottom of the character's capsule.
	if (HitResult.bBlockingHit)
	{
		return FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
	}

	/* If no ground was hit, and we're helplessly dangling over a bottomless void (which happens a lot in this game),
	 * return the maximum trace distance. */
	return MAX_GROUND_TRACE_DISTANCE;
}
