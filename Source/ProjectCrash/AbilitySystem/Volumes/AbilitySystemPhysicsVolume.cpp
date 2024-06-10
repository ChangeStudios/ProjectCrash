// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Volumes/AbilitySystemPhysicsVolume.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "DisplayDebugHelpers.h"
#include "GameFramework/HUD.h"
#include "EngineUtils.h"

AAbilitySystemPhysicsVolume::AAbilitySystemPhysicsVolume()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAbilitySystemPhysicsVolume::BeginPlay()
{
	Super::BeginPlay();

#if WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
	AHUD::OnShowDebugInfo.AddStatic(&AAbilitySystemPhysicsVolume::OnShowDebugInfo);
#endif // WITH_GAMEPLAY_DEBUGGER && WITH_EDITOR
}

void AAbilitySystemPhysicsVolume::ActorEnteredVolume(AActor* Other)
{
	Super::ActorEnteredVolume(Other);

	// Only utilize the ability system on the server.
	if (!HasAuthority())
	{
		return;
	}

	// Perform on-enter ability system logic when an actor with an ASC enters this volume.
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Other))
	{
		// Create a new handle for the entered actor. Used to cache handles for temporary abilities and effects.
		EnteredActorHandles.Add(Other);

		// Grant and cache temporary gameplay abilities.
		for (const TSubclassOf<UGameplayAbility> TemporaryAbility : OngoingAbilitiesToGive)
		{
			const FGameplayAbilitySpecHandle& TempAbilityHandle = ASC->GiveAbility(TemporaryAbility);
			if (TempAbilityHandle.IsValid())
			{
				EnteredActorHandles[Other].TemporarilyGrantedAbilities.Add(TempAbilityHandle);
			}
		}

		// Apply and cache temporary gameplay effects.
		for (const TSubclassOf<UGameplayEffect> TemporaryEffect : OngoingEffectsToApply)
		{
			// The entering actor is the instigator; this volume is the effect causer.
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(Other, this);

			// Create an outgoing spec to apply the effect.
			const FGameplayEffectSpecHandle& OutgoingSpecHandle = ASC->MakeOutgoingSpec(TemporaryEffect, 1, ContextHandle);
			if (OutgoingSpecHandle.IsValid())
			{
				// Apply the gameplay effect.
				FActiveGameplayEffectHandle TempEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*OutgoingSpecHandle.Data.Get());

				// Cache the effect if it was successfully applied to remove it when the actor leaves this volume.
				if (TempEffectHandle.WasSuccessfullyApplied())
				{
					EnteredActorHandles[Other].TemporarilyAppliedEffects.Add(TempEffectHandle);
				}
			}
		}

		// Grant permanent gameplay abilities.
		for (TSubclassOf<UGameplayAbility> PermanentAbility : PermanentAbilitiesToGive)
		{
			const FGameplayAbilitySpecHandle& PermanentAbilityHandle = ASC->GiveAbility(PermanentAbility);

			if (!PermanentAbilityHandle.IsValid())
			{
				ABILITY_LOG(Warning, TEXT("AAbilitySystemPhysicsVolume: Volume [%s] tried to grant permanent ability [%s], but was unsuccessful."), *GetName(), *GetNameSafe(PermanentAbility));
			}
		}

		// Fire gameplay events.
		for (const FGameplayTag& EventTag : GameplayEventsToSendOnEnter)
		{
			FGameplayEventData Payload;
			Payload.EventTag = EventTag;

			FScopedPredictionWindow NewScopedWindow(ASC, true);
			ASC->HandleGameplayEvent(EventTag, &Payload);
		}
	}
}

void AAbilitySystemPhysicsVolume::ActorLeavingVolume(AActor* Other)
{
	Super::ActorLeavingVolume(Other);

	// Only utilize the ability system on the server.
	if (!HasAuthority())
	{
		return;
	}

	// Perform on-exit ability system logic when an actor with an ASC exits this volume.
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Other))
	{
		// Remove temporary abilities and effects if the leaving actor has any.
		if (EnteredActorHandles.Find(Other))
		{
			// Remove temporary gameplay effects.
			for (const FActiveGameplayEffectHandle& TemporaryEffectHandle : EnteredActorHandles[Other].TemporarilyAppliedEffects)
			{
				ASC->RemoveActiveGameplayEffect(TemporaryEffectHandle);
			}

			// Remove temporary abilities.
			for (const FGameplayAbilitySpecHandle& TemporaryAbilityHandle : EnteredActorHandles[Other].TemporarilyGrantedAbilities)
			{
				ASC->ClearAbility(TemporaryAbilityHandle);
			}

			// Clear the leaving actor from the volume's actor cache.
			EnteredActorHandles.Remove(Other);
		}

		// Apply gameplay effects applied on exit.
		for (const TSubclassOf<UGameplayEffect> ExitEffect : OnExitEffectsToApply)
		{
			// The exiting actor is the instigator; this volume is the effect causer.
			FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			ContextHandle.AddInstigator(Other, this);

			// Create an outgoing spec to apply the effect.
			const FGameplayEffectSpecHandle& OutgoingSpecHandle = ASC->MakeOutgoingSpec(ExitEffect, 1, ContextHandle);
			if (OutgoingSpecHandle.IsValid())
			{
				// Apply the gameplay effect.
				FActiveGameplayEffectHandle ExitEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*OutgoingSpecHandle.Data.Get());

				if (!ExitEffectHandle.WasSuccessfullyApplied())
				{
					ABILITY_LOG(Warning, TEXT("AAbilitySystemPhysicsVolume: Volume [%s] tried to apply effect [%s] on exit, but was unsuccessful."), *GetName(), *GetNameSafe(ExitEffect));
				}
			}
		}

		// Fire gameplay events.
		for (const FGameplayTag& EventTag : GameplayEventsToSendOnExit)
		{
			FGameplayEventData Payload;
			Payload.EventTag = EventTag;

			FScopedPredictionWindow NewScopedWindow(ASC, true);
			ASC->HandleGameplayEvent(EventTag, &Payload);
		}
	}
}

#if WITH_EDITOR
void AAbilitySystemPhysicsVolume::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// Draw debug info for this actor if GAS debugging is enabled.
	if (DisplayInfo.IsDisplayOn(TEXT("AbilitySystem")))
	{
		const UWorld* World = HUD->GetWorld();
		for (TActorIterator<ThisClass> It(World); It; ++It)
		{
			const AAbilitySystemPhysicsVolume* GEVolume = *It;

			// Draw a debug shape around the volume each tick.
			const float DebugThickness = FMath::Clamp(GEVolume->GetBounds().BoxExtent.Length() / 500.0f, 5.0f, 100.0f);
			DrawDebugBox(World, GEVolume->GetActorLocation(), GEVolume->GetBounds().BoxExtent, FColor::Red, false, 0, 0, DebugThickness);
		}
	}
}
#endif // WITH_EDITOR
