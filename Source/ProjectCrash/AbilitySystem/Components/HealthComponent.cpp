// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Components/HealthComponent.h"

#include "AbilitySystemLog.h"
#include "CrashAbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "CrashStatics.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "GameFramework/CrashAssetManager.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Data/GlobalGameData.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	HealthSet = nullptr;

	DeathState = EDeathState::NotDead;
}

void UHealthComponent::InitializeWithAbilitySystem(UCrashAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	// Don't initialize this component if it's already been initialized with an ASC.
	if (AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize health component on [%s]. The component has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	// Don't initialize this component without a valid ASC.
	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize health component on [%s]. An invalid ability system component was given."), *GetNameSafe(Owner));
		return;
	}

	// Don't initialize this component if the given ASC does not have a HealthAttributeSet.
	HealthSet = AbilitySystemComponent->GetSet<UHealthAttributeSet>();
	if (!HealthSet)
	{
		ABILITY_LOG(Error, TEXT("Failed to initialize health component on [%s] with ASC owned by [%s]. The ASC does not have an attribute set of type HealthAttributeSet."), *GetNameSafe(Owner), *GetNameSafe(AbilitySystemComponent->GetOwnerActor()));
		return;
	}

	// Clear any leftover health tags ("Dying," "Dead," etc.). 
	ClearGameplayTags();

	// Register to listen for health attribute events.
	HealthSet->HealthAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnHealthChanged);
	HealthSet->MaxHealthAttributeChangedDelegate.AddDynamic(this, &ThisClass::OnMaxHealthChanged);
	HealthSet->OutOfHealthAttributeDelegate.AddUObject(this, &ThisClass::OnOutOfHealth);

	// Initialize the attributes to the attribute set's default values.
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetMaxHealthAttribute(), HealthSet->GetMaxHealth());
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealthAttribute(), HealthSet->GetMaxHealth());
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetDamageAttribute(), 0.0f);
	AbilitySystemComponent->SetNumericAttributeBase(UHealthAttributeSet::GetHealingAttribute(), 0.0f);

	// Broadcast the attributes' initialization.
	HealthChangedDelegate.Broadcast(this, nullptr, HealthSet->GetHealth(), HealthSet->GetHealth());
	MaxHealthChangedDelegate.Broadcast( this, nullptr, HealthSet->GetMaxHealth(), HealthSet->GetMaxHealth());
}

void UHealthComponent::UninitializeFromAbilitySystem()
{
	// Clear health event delegates.
	if (HealthSet)
	{
		HealthSet->HealthAttributeChangedDelegate.RemoveAll(this);
		HealthSet->MaxHealthAttributeChangedDelegate.RemoveAll(this);
		HealthSet->OutOfHealthAttributeDelegate.RemoveAll(this);
	}

	// Clear cached variables.
	HealthSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void UHealthComponent::OnUnregister()
{
	// Uninitialize this component when it's unregistered, in case UninitializeFromAbilitySystem wasn't called manually.
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

float UHealthComponent::GetHealth() const
{
	return (HealthSet ? HealthSet->GetHealth() : 0.0f);
}

float UHealthComponent::GetMaxHealth() const
{
	return (HealthSet ? HealthSet->GetMaxHealth() : 0.0f);
}

float UHealthComponent::GetHealthNormalized() const
{
	if (HealthSet)
	{
		const float Health = HealthSet->GetHealth();
		const float MaxHealth = HealthSet->GetMaxHealth();

		return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);
	}

	return 0.0f;
}

void UHealthComponent::OnHealthChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	HealthChangedDelegate.Broadcast(this, EffectInstigator, OldValue, NewValue);
}

void UHealthComponent::OnMaxHealthChanged(AActor* EffectInstigator, const FGameplayEffectSpec& EffectSpec, float OldValue, float NewValue)
{
	MaxHealthChangedDelegate.Broadcast(this, EffectInstigator, OldValue, NewValue);
}

void UHealthComponent::OnOutOfHealth(AActor* DamageInstigator, const FGameplayEffectSpec& DamageEffectSpec, float DamageMagnitude)
{
#if WITH_SERVER_CODE

	if (AbilitySystemComponent)
	{
		// Send a "Death" event to the dying actor's ASC. This can be used to trigger a death ability.
		{
			FGameplayEventData Payload;
			Payload.EventTag = CrashGameplayTags::TAG_GameplayEvent_Ability_Death;
			Payload.Instigator = DamageInstigator;
			Payload.Target = AbilitySystemComponent->GetAvatarActor();
			Payload.OptionalObject2 = DamageEffectSpec.Def; // Optional object is the damage effect definition.
			Payload.ContextHandle = DamageEffectSpec.GetEffectContext();
			Payload.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			Payload.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();
			Payload.EventMagnitude = DamageMagnitude;

			FScopedPredictionWindow NewScopedPredictionWindow(AbilitySystemComponent, true);
			AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
		}

		/* Broadcast a message communicating the death that other systems can observe. This is used for things like
		 * updating the kill-feed. */
		if (UGameplayMessageSubsystem::HasInstance(GetWorld()))
		{
			FCrashVerbMessage DeathMessage;
			DeathMessage.Verb = CrashGameplayTags::TAG_Message_Death;
			DeathMessage.Instigator = DamageInstigator;
			DeathMessage.InstigatorTags = *DamageEffectSpec.CapturedSourceTags.GetAggregatedTags();
			DeathMessage.Target = UCrashStatics::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			DeathMessage.TargetTags = *DamageEffectSpec.CapturedTargetTags.GetAggregatedTags();

			// Broadcast the message on the server.
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(DeathMessage.Verb, DeathMessage);

			// Broadcast the message to clients.
			if (ACrashGameState* GS = Cast<ACrashGameState>(UGameplayStatics::GetGameState(GetWorld())))
			{
				GS->MulticastReliableMessageToClients(DeathMessage);
			}
		}
	}
	else
	{
		ABILITY_LOG(Error, TEXT("UHealthComponent: Actor [%s] ran out of health, but could not die."), *GetNameSafe(GetOwner()));
	}

#endif // #if WITH_SERVER_CODE
}

void UHealthComponent::StartDeath()
{
	// Ensure valid transition order.
	if (DeathState != EDeathState::NotDead)
	{
		return;
	}

	// Locally update the current DeathState. This gets synced on clients by OnRep_DeathState.
	DeathState = EDeathState::DeathStarted;

	// Give the dying ASC a "Dying" tag during the death sequence.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(CrashGameplayTags::TAG_State_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	// Broadcast the death.
	DeathStartedDelegate.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void UHealthComponent::FinishDeath()
{
	// Ensure valid transition order.
	if (DeathState != EDeathState::DeathStarted)
	{
		return;
	}

	// Locally update the current DeathState. This gets synced on clients by OnRep_DeathState.
	DeathState = EDeathState::DeathFinished;

	// Add a "Dead" tag to the owner. This will be cleared when the ASC is re-initialized with a new health component.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(CrashGameplayTags::TAG_State_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	// Broadcast the death.
	DeathFinishedDelegate.Broadcast(Owner);
	Owner->ForceNetUpdate();
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	const EDeathState NewDeathState = DeathState;

	/* StartDeath and FinishDeath predictively change the death state from the Death gameplay ability. When the server
	 * updates it, we revert it back to the local death state here before we try to catch up. */
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back, but we've already predicted past the server state.
		ABILITY_LOG(Warning, TEXT("HealthComponent on actor [%s] predicted past server death state [%d] -> [%d]."), *GetNameSafe(GetOwner()), (uint8)OldDeathState, (uint8)NewDeathState);
		return;
	}

	if (OldDeathState == EDeathState::NotDead)
	{
		// If the server reached Dying before we start our death, catch up by starting the death.
		if (NewDeathState == EDeathState::DeathStarted)
		{
			StartDeath();
		}
		// If the server reached Dead before we started our death, catch up by starting AND finishing the death.
		else if (NewDeathState == EDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			ABILITY_LOG(Error, TEXT("HealthComponent on actor [%s] attempted invalid death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), (uint8)OldDeathState, (uint8)NewDeathState);
		}
	}
	else if (OldDeathState == EDeathState::DeathStarted)
	{
		// If the server reached Dead before we finished our death, catch up by finishing the death.
		if (NewDeathState == EDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			ABILITY_LOG(Error, TEXT("HealthComponent on actor [%s] attempted invalid death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), (uint8)OldDeathState, (uint8)NewDeathState);
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("HealthComponent on actor [%s] failed death transition [%d] -> [%d]."), *GetNameSafe(GetOwner()), (uint8)OldDeathState, (uint8)NewDeathState);
}

void UHealthComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(CrashGameplayTags::TAG_State_Dying, 0);
		AbilitySystemComponent->SetLooseGameplayTagCount(CrashGameplayTags::TAG_State_Dead, 0);
	}
}

void UHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	if ((DeathState == EDeathState::NotDead) && AbilitySystemComponent)
	{
		// Retrieve the damage gameplay effect class that we'll use to apply damage.
		const TSubclassOf<UGameplayEffect> DamageGE = UCrashAssetManager::GetOrLoadClass(UGlobalGameData::Get().DamageGameplayEffect_SetByCaller);

		if (!DamageGE)
		{
			if (UGlobalGameData::Get().DamageGameplayEffect_SetByCaller.IsNull())
			{
				UE_LOG(LogCrash, Error, TEXT("UHealthComponent::DamageSelfDestruct failed to find damage gameplay effect. DamageGameplayEffect_SetByCaller has not been set in global data."));
			}
			else
			{
				UE_LOG(LogCrash, Error, TEXT("UHealthComponent::DamageSelfDestruct failed to load damage gameplay effect [%s]."), *UGlobalGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
			}

			return;
		}

		// Create an effect spec for the damage.
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

		// If the actor fell out of the world, add the last person to deal knockback to them as an instigator.
		if (bFellOutOfWorld && AbilitySystemComponent->GetCurrentKnockbackSource())
		{
			EffectContext.AddInstigator(AbilitySystemComponent->GetCurrentKnockbackSource(), AbilitySystemComponent->GetCurrentKnockbackSource() /* The world is technically the effect causer, but this needs to be valid for the damage execution. */);
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1.0f, EffectContext);
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (!Spec)
		{
			UE_LOG(LogCrash, Error, TEXT("UHealthComponent::DamageSelfDestruct failed to apply damage to owner [%s]: unable to make outgoing spec."), *GetNameSafe(GetOwner()));
		}

		// Add context tags to the effect.
		Spec->AddDynamicAssetTag(CrashGameplayTags::TAG_GameplayEffects_Damage_SelfDestruct);

		if (bFellOutOfWorld)
		{
			Spec->AddDynamicAssetTag(CrashGameplayTags::TAG_GameplayEffects_Damage_FellOutOfWorld);
		}

		// Set damage magnitude.
		const float DamageMagnitude = GetMaxHealth();
		Spec->SetSetByCallerMagnitude(CrashGameplayTags::TAG_GameplayEffects_SetByCaller_Damage, DamageMagnitude);

		// Apply the effect.
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, DeathState);
}
