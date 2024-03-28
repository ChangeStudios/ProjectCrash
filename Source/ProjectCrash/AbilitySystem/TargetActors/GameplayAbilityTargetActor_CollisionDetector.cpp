// Copyright Samuel Reitich 2024.


#include "GameplayAbilityTargetActor_CollisionDetector.h"

#include "Components/ShapeComponent.h"

AGameplayAbilityTargetActor_CollisionDetector::AGameplayAbilityTargetActor_CollisionDetector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	
	CollisionDetector = nullptr;
}

void AGameplayAbilityTargetActor_CollisionDetector::BeginPlay()
{
	CollisionDetector->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCollisionBegin);

	Super::BeginPlay();
}

void AGameplayAbilityTargetActor_CollisionDetector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGameplayAbilityTargetActor_CollisionDetector::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
}

void AGameplayAbilityTargetActor_CollisionDetector::ConfirmTargetingAndContinue()
{
	Super::ConfirmTargetingAndContinue();
}

void AGameplayAbilityTargetActor_CollisionDetector::OnCollisionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}
