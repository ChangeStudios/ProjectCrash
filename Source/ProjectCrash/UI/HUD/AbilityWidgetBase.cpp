// Copyright Samuel Reitich. All rights reserved.

#include "UI/HUD/AbilityWidgetBase.h"

#include "AbilitySystemLog.h"
#include "CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "GameFramework/PlayerState.h"

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

	if (FGameplayAbilitySpec* AbilitySpec = BoundASC->FindAbilitySpecFromClass(AbilityClass.Get()))
	{
		// Use instanced abilities' primary instance.
		UGameplayAbility* AbilityInstance = AbilitySpec->GetPrimaryInstance();

		// Fall back to using the CDO for non-instanced abilities.
		if (!IsValid(AbilityInstance))
		{
			AbilityInstance = AbilitySpec->Ability;
		}

		// Cache the bound ability instance and its spec.
		if (UCrashGameplayAbilityBase* CrashAbilityInstance = Cast<UCrashGameplayAbilityBase>(AbilityInstance))
		{
			BoundAbility = CrashAbilityInstance;
			BoundSpecHandle = AbilitySpec->Handle;
			return;
		}
	}

	ABILITY_LOG(Error, TEXT("Owning player (%s) of ability widget (%s) does not have an ability of class (%s). Ability widgets should only be created by the ability to which they should be bound."), *GetNameSafe(GetOwningPlayer()), *GetNameSafe(GetClass()), *GetNameSafe(AbilityClass.Get()));
}

void UAbilityWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bWantsDisabledEvents)
	{
		if (BoundASC.IsValid() && BoundAbility.IsValid())
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

void UAbilityWidgetBase::OnAbilityMessageReceived(FGameplayTag Channel, const FCrashAbilityMessage& Message)
{
	// We only care about OUR abilities.
	if (!Message.ActorInfo.OwnerActor.IsValid() ||
		 Message.ActorInfo.OwnerActor.Get() != GetOwningPlayerState())
	{
		return;
	}

	// TODO: This is failing when starting levels.
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

	// Forward ability messages to their corresponding events, filtering for messages related to the bound ability.
	if (Message.AbilitySpecHandle == BoundSpecHandle)
	{
		if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Activated_Success))
		{
			K2_OnAbilityActivated_Success(BoundAbility.Get());
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Ended))
		{
			K2_OnAbilityEnded(BoundAbility.Get());
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Started))
		{
			K2_OnAbilityCooldownStarted(BoundAbility.Get(), Message.Magnitude);
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_Cooldown_Ended))
		{
			K2_OnAbilityCooldownEnded(BoundAbility.Get());
		}
		else if (Message.MessageType.MatchesTag(CrashGameplayTags::TAG_Message_Ability_CostChanged))
		{
			K2_OnAbilityCostChanged(BoundAbility.Get(), Message.Magnitude);
		}
	}
}
