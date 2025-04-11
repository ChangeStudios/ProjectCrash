// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/AbilitySystemUtilitiesLibrary.h"

#include "CrashAbilitySystemGlobals.h"
#include "CrashGameplayAbilityTypes.h"
#include "Components/CrashAbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

/** When knockback is applied to a grounded character, a small upward force will be added so they don't slide on the
 * ground. The upward force will be min-clamped to (MIN_UPWARD_KNOCKBACK_PCT * (2D knockback force length)). */
#define MIN_UPWARD_KNOCKBACK_PCT 0.325

bool UAbilitySystemUtilitiesLibrary::HasLineOfSight(const UObject* WorldContextObject, AActor* SourceActor, AActor* TargetActor, ETraceTypeQuery TraceChannel, bool bIgnorePawns)
{
	if (!ensure(SourceActor && TargetActor))
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);

	// Always ignore the actors performing the LOS trace.
	TArray<AActor*> ActorsToIgnore = { SourceActor, TargetActor };

	// Ignore every pawn in the level, if desired.
	if (bIgnorePawns)
	{
		TArray<AActor*> OutPawns;
		UGameplayStatics::GetAllActorsOfClass(WorldContextObject, APawn::StaticClass(), OutPawns);
		ActorsToIgnore.Append(OutPawns);
	}

	// Collect data for LOS trace.
	FHitResult OutHitResult;
	FVector Start = SourceActor->GetActorLocation();
	FVector End = TargetActor->GetActorLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(ActorsToIgnore);

	// Perform trace.
	World->LineTraceSingleByChannel(OutHitResult, Start, End, UEngineTypes::ConvertToCollisionChannel(TraceChannel), QueryParams);

	// Return if the trace wasn't blocked.
	return !OutHitResult.bBlockingHit;
}

void UAbilitySystemUtilitiesLibrary::ApplyKnockbackToTargetFromLocation(float Force, FVector Source, AActor* Target, AActor* Instigator, const UGameplayEffect* SourceEffect)
{
	if (!ensure(Target))
	{
		return;
	}

	// Calculate direction.
	const FQuat DirectionRot = UKismetMathLibrary::FindLookAtRotation(Source, Target->GetActorLocation()).Quaternion();
	const FVector ForceVector = Force * DirectionRot.Vector();

	// Apply knockback.
	ApplyKnockbackToTargetInDirection(ForceVector, Target, Instigator, SourceEffect);
}

void UAbilitySystemUtilitiesLibrary::ApplyKnockbackToTargetInDirection(FVector Velocity, AActor* Target, AActor* Instigator, const UGameplayEffect* SourceEffect)
{
	// If the target actor is a character, launch them via their movement component.
	if (ACharacter* TargetChar = Cast<ACharacter>(Target))
	{
		UCharacterMovementComponent* TargetMovementComp = TargetChar->GetCharacterMovement();
		if (ensure(IsValid(TargetMovementComp)))
		{
			// If the character is on the ground, add a small upward force so ground friction doesn't stop the impulse.
			if (TargetMovementComp->IsMovingOnGround())
			{
				Velocity.Z = FMath::Max(Velocity.Size2D() * MIN_UPWARD_KNOCKBACK_PCT, Velocity.Z);
			}
		}

		TargetChar->LaunchCharacter(Velocity, true, true);
	}
	// If the target actor has a primitive root with physics enabled, add an impulse to their root.
	else if (UPrimitiveComponent* RootScene = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		if (RootScene->IsAnySimulatingPhysics())
		{
			// NOTE: May have to rescale force to take mass into account.
			RootScene->AddImpulse(Velocity);
		}
	}

	/* If the target has an ASC, track the current source of the target's knockback. This is cleared when the target
	 * lands on the ground. */
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(Target))
	{
		CrashASC->SetCurrentKnockbackSource(Instigator, SourceEffect);
	}
}

FHitResult UAbilitySystemUtilitiesLibrary::GetTargetDataHitResultWithCustomDirection(const FGameplayAbilityTargetDataHandle& TargetData, FVector NewDirection)
{
	if (TargetData.Data.IsValidIndex(0))
	{
		FGameplayAbilityTargetData* Data = TargetData.Data[0].Get();
		if (Data)
		{
			if (const FHitResult* HitResultPtr = Data->GetHitResult())
			{
				// Copy the target data's hit result.
				FHitResult HitResult = FHitResult(*HitResultPtr);
				HitResult.Normal = NewDirection;
				return HitResult;
			}
		}
	}

	return FHitResult();
}

FGameplayTargetDataFilterHandle UAbilitySystemUtilitiesLibrary::MakeCrashFilterHandle(FCrashTargetDataFilter Filter, AActor* FilterActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FCrashTargetDataFilter* NewFilter = new FCrashTargetDataFilter(Filter);
	NewFilter->InitializeFilterContext(FilterActor);
	FilterHandle.Filter = TSharedPtr<FCrashTargetDataFilter>(NewFilter);
	return FilterHandle;
}
