// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/TargetActors/GameplayAbilityTargetActor_CollisionDetector_Capsule.h"

#include "DisplayDebugHelpers.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/HUD.h"

AGameplayAbilityTargetActor_CollisionDetector_Capsule::AGameplayAbilityTargetActor_CollisionDetector_Capsule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CapsuleRadius = 45.0f;
	CapsuleHalfHeight = 90.0f;

	// Construct this target actor's collision detection component.
	CollisionDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule"));
	DetectorAsCapsule = CollisionDetector ? Cast<UCapsuleComponent>(CollisionDetector) : nullptr;
	RootComponent = CollisionDetector ? CollisionDetector : nullptr;
}

void AGameplayAbilityTargetActor_CollisionDetector_Capsule::BeginPlay()
{
	Super::BeginPlay();

#if WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
	AHUD::OnShowDebugInfo.AddStatic(&AGameplayAbilityTargetActor_CollisionDetector_Capsule::OnShowDebugInfo);
#endif // WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
}

void AGameplayAbilityTargetActor_CollisionDetector_Capsule::Configure(float InCapsuleRadius, float InCapsuleHalfHeight, FName InCollisionProfile, bool bInIgnoreSelf, bool bInRepeatTargets, bool bInResetTargetsOnStart, FGameplayTargetDataFilterHandle InFilter, bool bInFilterForGASActors, FGameplayTagContainer IgnoreTargetsWithTags, bool bInShouldProduceTargetDataOnServer)
{
	check(DetectorAsCapsule);

	// Update this target actor's data.
	CapsuleRadius = InCapsuleRadius;
	CapsuleHalfHeight = InCapsuleHalfHeight;
	CollisionProfile = InCollisionProfile;
	bIgnoreSelf = bInIgnoreSelf;
	bRepeatTargets = bInRepeatTargets;
	bResetTargetsOnStart = bInResetTargetsOnStart;
	Filter = InFilter;
	bFilterForGASActors = bInFilterForGASActors;
	IgnoredTargetTags = IgnoreTargetsWithTags;
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;

	DetectorAsCapsule->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
}

#if WITH_EDITOR
void AGameplayAbilityTargetActor_CollisionDetector_Capsule::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// Draw debug info for this actor if GAS debugging is enabled.
	if (DisplayInfo.IsDisplayOn(TEXT("AbilitySystem")))
	{
		const UWorld* World = HUD->GetWorld();
		for (TActorIterator<ThisClass> It(World); It; ++It)
		{
			const AGameplayAbilityTargetActor_CollisionDetector_Capsule* TargetCapsule = *It;

			// Draw a debug shape of the target actor each tick.
			DrawDebugCapsule(World, TargetCapsule->GetActorLocation(), TargetCapsule->CapsuleHalfHeight, TargetCapsule->CapsuleRadius, TargetCapsule->GetActorRotation().Quaternion(), FColor::Green, false, 0, 0, 1.0f);
		}
	}
}
#endif // WITH_EDITOR