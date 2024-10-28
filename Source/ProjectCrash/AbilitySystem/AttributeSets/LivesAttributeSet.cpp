// Copyright Samuel Reitich. All rights reserved.

#include "AbilitySystem/AttributeSets/LivesAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "GameplayEffect.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Teams/TeamSubsystem.h"

void ULivesAttributeSet::PostInitProperties()
{
	Super::PostInitProperties();

	if (GetWorld() && GetOwningActor()->HasAuthority())
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		DeathListener = MessageSubsystem.RegisterListener(CrashGameplayTags::TAG_Message_Player_Death, this, &ULivesAttributeSet::OnPlayerDeath);
	}

	// TODO: Init using game mode properties.
}

ULivesAttributeSet::ULivesAttributeSet()
{
}

void ULivesAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void ULivesAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void ULivesAttributeSet::OnPlayerDeath(FGameplayTag Channel, const FCrashVerbMessage& Message)
{
	if (UTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UTeamSubsystem>())
	{
		int32 DeadPlayerTeam = TeamSubsystem->FindTeamFromObject(Message.Instigator);
		int32 OwningPlayerTeam = TeamSubsystem->FindTeamFromObject(GetOwningActor());

		if (DeadPlayerTeam == OwningPlayerTeam)
		{
			UGameplayEffect* GE_RemoveLife = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("RemoveLife")));
			GE_RemoveLife->DurationPolicy = EGameplayEffectDurationType::Instant;

			int32 Idx = GE_RemoveLife->Modifiers.Num();
			GE_RemoveLife->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& LifeMod = GE_RemoveLife->Modifiers[Idx];
			LifeMod.ModifierMagnitude = FScalableFloat(-1.0f);
			LifeMod.ModifierOp = EGameplayModOp::Additive;
			LifeMod.Attribute = GetLivesAttribute();

			UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponentChecked();
			ASC->ApplyGameplayEffectToSelf(GE_RemoveLife, 1.0f, ASC->MakeEffectContext());
		}
	}
}
