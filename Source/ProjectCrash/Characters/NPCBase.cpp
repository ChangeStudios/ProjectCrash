// Copyright Samuel Reitich 2024.


#include "Characters/NPCBase.h"

#include "AbilitySystemLog.h"
#include "Data/NPCData.h"
#include "AbilitySystem/AttributeSets/HealthAttributeSet.h"
#include "AbilitySystem/Components/AbilitySystemExtensionComponent.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"

ANPCBase::ANPCBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Networking.
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	// Create the ability system component.
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UCrashAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	NetUpdateFrequency = 100.0f; // The ASC Needs to be updated at a high frequency.

	// Create the ASC extension component to initialize and uninitialize the ASC.
	ASCExtensionComponent = CreateDefaultSubobject<UAbilitySystemExtensionComponent>(TEXT("AbilitySystemExtensionComponent"));
	ASCExtensionComponent->OnAbilitySystemInitialized_RegisterAndCall(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	ASCExtensionComponent->OnAbilitySystemUninitialized_Register(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));

	// Create this character's attribute sets.
	HealthSet = CreateDefaultSubobject<UHealthAttributeSet>(TEXT("HealthSet"));

	// Create this character's attribute management components.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ANPCBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize the ASC with this character.
	check(AbilitySystemComponent);
	if (ASCExtensionComponent)
	{
		ASCExtensionComponent->InitializeAbilitySystem(AbilitySystemComponent, this);
	}
	// If the extension component is not yet initialized, the ASC actor info can be directly initialized.
	else
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		ABILITY_LOG(Warning, TEXT("ASC for [%s] could not initialize using its ASC extension component. Fell back to direct actor info initialization."), *GetName());
	}
}

UAbilitySystemComponent* ANPCBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UCrashAbilitySystemComponent* ANPCBase::GetCrashAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC ? Cast<UCrashAbilitySystemComponent>(ASC) : nullptr;
}

void ANPCBase::OnAbilitySystemInitialized()
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent();
	check(CrashASC);

	if (!IsValid(NPCData))
	{
		ABILITY_LOG(Error, TEXT("[%s]'s ASC could not be properly initialized: Missing NPCData asset reference."), *GetName());
		return;
	}

	// Initialize this character's attribute sets.
	HealthComponent->InitializeWithAbilitySystem(CrashASC, NPCData->HealthAttributeBaseValues);
}

void ANPCBase::OnAbilitySystemUninitialized()
{
	UCrashAbilitySystemComponent* CrashASC = GetCrashAbilitySystemComponent();
	check(CrashASC);

	// Uninitialize this character's attribute sets.
	HealthComponent->UninitializeFromAbilitySystem();
}
