// Copyright Samuel Reitich. All rights reserved.

#include "Characters/PushingCapsuleComponent.h"

#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

UPushingCapsuleComponent::UPushingCapsuleComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), PushStrength(10000.0f)
{
}

void UPushingCapsuleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PushStrength > 0.0f)
	{
		if (GetOwner()->HasAuthority())
		{
			OnComponentBeginOverlap.AddUniqueDynamic(this, &UPushingCapsuleComponent::OnCollisionBegin);
			OnComponentEndOverlap.AddUniqueDynamic(this, &UPushingCapsuleComponent::OnCollisionEnd);
			TickCollider();
		}
	}
}

void UPushingCapsuleComponent::TickCollider()
{
	if (AActor* Owner = GetOwner())
	{
		for (APawn* CollidingPawn : CollidingPawns)
		{
			if (UCharacterMovementComponent* MovementComp = CollidingPawn->GetComponentByClass<UCharacterMovementComponent>())
			{
				const FVector OwnerLoc = Owner->GetActorLocation();
				const FVector ColliderLoc = CollidingPawn->GetActorLocation();
				const FRotator Direction = UKismetMathLibrary::FindLookAtRotation(OwnerLoc, ColliderLoc);
				const FVector DirectionVec = Direction.Vector();
				const FVector DirectionVec2D = FVector(DirectionVec.X, DirectionVec.Y, 0.0f);
				const float AirborneMultiplier = MovementComp->IsFalling() ? 0.5f : 1.0f;
				MovementComp->AddImpulse(DirectionVec2D * PushStrength * AirborneMultiplier);
			}
		}

		Owner->GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			TickCollider();
		}));
	}
}

void UPushingCapsuleComponent::OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* OtherActorAsPawn = Cast<APawn>(OtherActor))
	{
		CollidingPawns.Add(OtherActorAsPawn);
	}
}

void UPushingCapsuleComponent::OnCollisionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* OtherActorAsPawn = Cast<APawn>(OtherActor))
	{
		CollidingPawns.Remove(OtherActorAsPawn);
	}
}