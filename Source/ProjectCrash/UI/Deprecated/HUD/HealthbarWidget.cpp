// Copyright Samuel Reitich. All rights reserved.


#include "UI/Deprecated/HUD/HealthbarWidget.h"

#include "AbilitySystemLog.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "AbilitySystem/Components/HealthComponent.h"
#include "GameFramework/CrashLogging.h"

void UHealthbarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HealthComponent = nullptr;

	// // If this widget's owning player has already initialized their ASC, initialize it with its owning player's pawn.
	// if (APawn* PossessedPawn = OwningASC ? GetOwningPlayer()->GetPawn() : nullptr)
	// {
	// 	InitWidget(PossessedPawn);
	// }
	// /* If this widget's owning player has not initialized their ASC yet, wait until they do to initialize this widget
	//  * so the health component functions correctly. */
	// else
	// {
	// 	ASCReadyDelegate.AddUObject(this, &UHealthbarWidget::OnASCReady);
	// }

	/* Bind a re-initialization callback to whenever this widget's owner's pawn changes in order to update the health
	 * component currently being used. */
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UHealthbarWidget::OnPawnChanged);
}

void UHealthbarWidget::InitWidget(APawn* PawnToBindTo)
{
	check(PawnToBindTo);

	// If this widget was already bound to a health component, unbind it.
	if (HealthComponent)
	{
		HealthComponent->HealthChangedDelegate.RemoveDynamic(this, &UHealthbarWidget::OnHealthChanged);
		HealthComponent->MaxHealthChangedDelegate.RemoveDynamic(this, &UHealthbarWidget::OnMaxHealthChanged);

		HealthComponent = nullptr;
	}

	// Bind this widget to the new health component.
	if (OwningASC)
	{
		// Cache the new health component.
		HealthComponent = UHealthComponent::FindHealthComponent(PawnToBindTo);

		if (!HealthComponent)
		{
			ABILITY_LOG(Error, TEXT("UHealthbarWidget: Tried to initialize a healthbar widget with a pawn that does not have a health component: [%s]."), *GetNameSafe(PawnToBindTo));
			return;
		}

		// Bind callbacks to the new health component's events.
		HealthComponent->HealthChangedDelegate.AddDynamic(this, &UHealthbarWidget::OnHealthChanged);
		HealthComponent->MaxHealthChangedDelegate.AddDynamic(this, &UHealthbarWidget::OnMaxHealthChanged);

		// Initialize this widget's Health and MaxHealth.
		OnHealthChanged(HealthComponent, GetOwningPlayer(), 0.0f, HealthComponent->GetHealth());
		OnMaxHealthChanged(HealthComponent, GetOwningPlayer(), 0.0f, HealthComponent->GetMaxHealth());
	}
}

void UHealthbarWidget::OnPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn) && UHealthComponent::FindHealthComponent(NewPawn))
	{
		// Re-initialize this widget with the new pawn.
		InitWidget(NewPawn);
	}
}

void UHealthbarWidget::OnASCReady()
{
	/* Initialize this widget with its owning player's pawn after its ASC has been initialized, to ensure the health
	 * component functions properly. */
	if (APawn* PossessedPawn = OwningASC ? GetOwningPlayer()->GetPawn() : nullptr)
	{
		InitWidget(PossessedPawn);
	}
}

void UHealthbarWidget::OnHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue)
{
	K2_OnHealthChanged(InHealthComponent, Instigator, OldValue, NewValue);
}

void UHealthbarWidget::OnMaxHealthChanged(UHealthComponent* InHealthComponent, AActor* Instigator, float OldValue, float NewValue)
{
	K2_OnMaxHealthChanged(InHealthComponent, Instigator, OldValue, NewValue);
}

void UHealthbarWidget::OnOutOfHealth(UHealthComponent* InHealthComponent, AActor* EffectInstigator, float DamageMagnitude)
{
	K2_OnOutOfHealth(InHealthComponent, EffectInstigator, DamageMagnitude);
}
