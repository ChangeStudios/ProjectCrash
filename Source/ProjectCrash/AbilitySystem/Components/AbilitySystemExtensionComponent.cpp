// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Components/AbilitySystemExtensionComponent.h"

#include "CrashAbilitySystemComponent.h"
#include "CrashGameplayTags.h"

UAbilitySystemExtensionComponent::UAbilitySystemExtensionComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
}

void UAbilitySystemExtensionComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that this component is in a pawn class.
	const APawn* Pawn = GetOwner<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("AbilitySystemExtensionComponent on [%s] can only be added to pawn actors."), *GetNameSafe(GetOwner()));

	// Ensure that this is the only instance of this component in its owner.
	TArray<UActorComponent*> ASCExtensionComponents;
	Pawn->GetComponents(UAbilitySystemExtensionComponent::StaticClass(), ASCExtensionComponents);
	ensureAlwaysMsgf((ASCExtensionComponents.Num() == 1), TEXT("Only one AbilitySystemExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));
}

void UAbilitySystemExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Uninitialize this component's owning pawn from its current ASC before ending play.
	UninitializeAbilitySystem();

	Super::EndPlay(EndPlayReason);
}

void UAbilitySystemExtensionComponent::InitializeAbilitySystem(UCrashAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	// Don't re-initialize the ASC if it hasn't changed.
	if (AbilitySystemComponent == InASC)
	{
		return;
	}

	// If this component's owning pawn is already the avatar of another ASC, uninitialize it from the old ASC.
	if (AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetOwner<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
	{
		/* If the given ASC has a different avatar already, uninitialize it from the ASC. This can happen on clients
		 * when lagging: the new pawn is spawned and possessed before the old one is removed. */
		if (ExistingAvatar->HasAuthority())
		{
			if (UAbilitySystemExtensionComponent* OtherExtensionComponent = FindAbilitySystemExtensionComponent(ExistingAvatar))
			{
				OtherExtensionComponent->UninitializeAbilitySystem();
			}
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	OnAbilitySystemInitialized.Broadcast();
}

void UAbilitySystemExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	/* Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the
	 * avatar actor. */
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		// Remove all ongoing gameplay cues.
		AbilitySystemComponent->RemoveAllGameplayCues();

		// Reset the ASC's avatar if it still has an owner.
		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		/* If the ASC doesn't have a valid owner, clear all of its actor info, not just its avatar. If the ASC doesn't
		 * have a valid actor owner, then its information regarding that owner is obsolete and should be reset. */
		else
		{
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	// Clear the cached ASC.
	AbilitySystemComponent = nullptr;
}

void UAbilitySystemExtensionComponent::HandleControllerChanged()
{
	// If this pawn's controller changed while the pawn is the avatar of an ASC, 
	if (AbilitySystemComponent && (AbilitySystemComponent->GetAvatarActor() == GetOwner<APawn>()))
	{
		// Ensure the ASC's actor info is still correct.
		ensure(AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor());

		// If the ASC's owner has been cleared, remove this component's owning pawn as the avatar.
		if (AbilitySystemComponent->GetOwnerActor() == nullptr)
		{
			UninitializeAbilitySystem();
		}
		// If the ASC still has the correct owner, refresh its actor info to reflect the pawn's new controller.
		else
		{
			AbilitySystemComponent->RefreshAbilityActorInfo();
		}
	}
}

void UAbilitySystemExtensionComponent::OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// Bind this delegate if it is not already bound.
	if (!OnAbilitySystemUninitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemUninitialized.Add(Delegate);
	}
}

void UAbilitySystemExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate Delegate)
{
	// Bind this delegate if it is not already bound.
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
    {
    	OnAbilitySystemInitialized.Add(Delegate);
    }

	// Execute the delegate.
    if (AbilitySystemComponent)
    {
    	Delegate.Execute();
    }
}
