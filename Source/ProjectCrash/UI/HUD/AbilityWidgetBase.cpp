// Copyright Samuel Reitich. All rights reserved.


#include "UI/HUD/AbilityWidgetBase.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "GameFramework/PlayerState.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "AbilityWidgetBase"

bool UAbilityWidgetBase::Initialize()
{
	if (!IsDesignTime())
	{
		// The Message.Ability tag is implicit, so we need to retrieve it manually.
		const FGameplayTag TAG_Message_Ability = CrashGameplayTags::TAG_Message_Ability_Added.GetTag().RequestDirectParent();

		// Start listening for messages related to the bound ability.
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		AbilityMessageListener = MessageSystem.RegisterListener<FCrashAbilityMessage>(TAG_Message_Ability, [&](FGameplayTag Channel, const FCrashAbilityMessage& Message)
		{
			OnAbilityMessageReceived(TAG_Message_Ability, Message);
		}, EGameplayMessageMatch::PartialMatch);

		/* "Ability Disabled" events are triggered by checking the ability's state each tick. If we're not using these
		 * events, don't bother checking for them to avoid unnecessary performance hits. */
		const UFunction* DisabledFunction = GetClass()->FindFunctionByName(FName("K2_OnAbilityDisabled"));
		const UFunction* EnabledFunction = GetClass()->FindFunctionByName(FName("K2_OnAbilityEnabled"));
		bWantsDisabledEvents = (DisabledFunction && DisabledFunction->Script.Num()) || (EnabledFunction && EnabledFunction->Script.Num());
	}

	return Super::Initialize();
}

void UAbilityWidgetBase::RemoveFromParent()
{
	if (AbilityMessageListener.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(World);
			MessageSystem.UnregisterListener(AbilityMessageListener); 
		}
	}

	Super::RemoveFromParent();
}

void UAbilityWidgetBase::OnAbilitySystemBound()
{
	Super::OnAbilitySystemBound();

	for (FGameplayAbilitySpec& AbilitySpec : BoundASC->GetActivatableAbilities())
	{
		// Use instanced abilities' primary instance.
		UGameplayAbility* Ability = AbilitySpec.GetPrimaryInstance();

		// Fall back to using the CDO.
		if (!IsValid(Ability))
		{
			Ability = AbilitySpec.Ability;
		}

		if (UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability))
		{
			if (CrashAbility->AbilityTags.HasTag(AbilityIdentifierTag))
			{
				BoundAbility = CrashAbility;
				BoundSpecHandle = AbilitySpec.Handle;
				return;
			}
		}
	}

	ABILITY_LOG(Error, TEXT("Owning player (%s) of ability widget does not have an ability with a matching identifying tag: (%s)."), *GetNameSafe(GetOwningPlayer()), *AbilityIdentifierTag.ToString());
}

void UAbilityWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bWantsDisabledEvents)
	{
		if (BoundASC.IsValid() && BoundAbility)
		{
			if (BoundAbility->AbilityTags.HasTag(AbilityIdentifierTag))
			{
				if (bIsAbilityEnabled != BoundAbility->CheckCanActivateAbility(BoundSpecHandle, BoundASC->AbilityActorInfo.Get()))
				{
					bIsAbilityEnabled = !bIsAbilityEnabled;

					if (bIsAbilityEnabled)
					{
						K2_OnAbilityEnabled(Cast<UCrashGameplayAbilityBase>(BoundAbility));
					}
					else
					{
						K2_OnAbilityDisabled(Cast<UCrashGameplayAbilityBase>(BoundAbility));
					}
				}
			}
		}
	}
}

void UAbilityWidgetBase::OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message)
{
	// We only care about OUR abilities.
	if (!Message.ActorInfo.OwnerActor.IsValid() ||
		 Message.ActorInfo.OwnerActor.Get() != GetOwningPlayerState())
	{
		return;
	}

	if (!ensure(BoundASC.IsValid()))
	{
		return;
	}

	if (!BoundSpecHandle.IsValid())
	{
		return;
	}

	if (!ensure(Message.AbilitySpecHandle.IsValid()))
	{
		return;
	}

	// const FGameplayAbilitySpec* AbilitySpec = BoundASC->FindAbilitySpecFromHandle(Message.AbilitySpecHandle);
	// UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
	// if (!IsValid(Ability))
	// {
	// 	Ability = AbilitySpec->Ability;
	// }
	// UCrashGameplayAbilityBase* CrashAbility = Cast<UCrashGameplayAbilityBase>(Ability);

	// Forward ability messages to their corresponding events, filtering for messages related to the specified ability.
	if (Message.AbilitySpecHandle == BoundSpecHandle)
	{
		if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Success))
		{
			K2_OnAbilityActivated_Success(BoundAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Ended))
		{
			K2_OnAbilityEnded(BoundAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started))
		{
			K2_OnAbilityCooldownStarted(BoundAbility, Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended))
		{
			K2_OnAbilityCooldownEnded(BoundAbility);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_CostChanged))
		{
			K2_OnAbilityCostChanged(BoundAbility, Message.Magnitude);
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UAbilityWidgetBase::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	const UFunction* DisabledFunction = GetClass()->FindFunctionByName(FName("K2_OnAbilityDisabled"));
	const UFunction* EnabledFunction = GetClass()->FindFunctionByName(FName("K2_OnAbilityEnabled"));
	const bool bIsUsingDisabledFunctions = (DisabledFunction && DisabledFunction->Script.Num()) || (EnabledFunction && EnabledFunction->Script.Num());

	// We're using "Ability Disabled" events, but not triggering them.
	if (bIsUsingDisabledFunctions && !bWantsDisabledEvents)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("UsingDisabledEvents", "Widget class {0} is using OnAbilityDisabled and/ OnAbilityEnabled, but WantsDisabledEvents is set to false. These events will never trigger!"), FText::FromString(GetNameSafe(this))));
	}
	// We're triggering "Ability Disabled" events, but not using them.
	else if (!bIsUsingDisabledFunctions && bWantsDisabledEvents)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(FText::Format(LOCTEXT("NotUsingDisabledEvents", "Widget class {0} set WantsDisabledEvents to true, but is not using OnAbilityDisabled and/ OnAbilityEnabled. Set WantsDisabledEvents to avoid unnecessary performance loss."), FText::FromString(GetNameSafe(this))));
	}

	return Result;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE