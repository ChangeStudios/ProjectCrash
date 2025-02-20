// Copyright Samuel Reitich. All rights reserved.


#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"

#include "AbilitySystem/CrashGameplayAbilityTypes.h"
#include "AbilitySystem/CrashGlobalAbilitySubsystem.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Characters/CrashCharacter.h"
#include "CrashGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashAbilityMessage.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "Characters/CrashCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CrashLogging.h"
#include "GameFramework/GameModes/CrashGameState.h"
#include "Kismet/GameplayStatics.h"

UCrashAbilitySystemComponent::UCrashAbilitySystemComponent()
{
	CurrentExclusiveAbility = nullptr;
	CurrentKnockbackSource = nullptr;
}

void UCrashAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	// Whether this ASC (A) has a valid pawn avatar and (B) that avatar is not the same as the previous avatar.
	const bool bHasValidNewAvatar = (Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor));

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// Notify this ASC and its abilities if a valid new avatar was set.
	if (bHasValidNewAvatar)
	{
		// Perform any necessary avatar initialization with the ASC.
		OnNewAvatarSet();

		// Notify each ability that a valid new avatar was set.
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			UCrashGameplayAbilityBase* CrashAbilityCDO = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability);

			if (!CrashAbilityCDO)
			{
				continue;
			}

			// Notify non-instanced abilities' CDO.
			if (CrashAbilityCDO->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
			{
				CrashAbilityCDO->OnNewAvatarSet();
			}
			// Notify each instance of instanced abilities.
			else
			{
				for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
				{
					if (UCrashGameplayAbilityBase* CrashAbilityInstance = Cast<UCrashGameplayAbilityBase>(AbilityInstance))
					{
						CrashAbilityInstance->OnNewAvatarSet();
					}
				}
			}
		}

		// Register this ASC with the global ability subsystem.
		if (UCrashGlobalAbilitySubsystem* GlobalAbilitySubsystem = UWorld::GetSubsystem<UCrashGlobalAbilitySubsystem>(GetWorld()))
		{
			GlobalAbilitySubsystem->RegisterASC(this);
		}

		// Attempt to activate any passive abilities when a valid new avatar is set.
		TryActivatePassiveAbilities();
	}
}

void UCrashAbilitySystemComponent::OnNewAvatarSet()
{
	// Clear any leftover knockback source from the last avatar.
	CurrentKnockbackSource = nullptr;

	// If the new avatar has a movement component, clear their knockback source whenever they land on the ground.
	if (UCrashCharacterMovementComponent* CrashCharMovementComp = UCrashCharacterMovementComponent::FindCrashMovementComponent(GetAvatarActor()))
	{
		CrashCharMovementComp->LandedDelegate.AddWeakLambda(this, [this](FHitResult Hit)
		{
			CurrentKnockbackSource = nullptr;
		});
	}
}

void UCrashAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister this ASC from the global ability subsystem.
	if (UCrashGlobalAbilitySubsystem* GlobalAbilitySubsystem = UWorld::GetSubsystem<UCrashGlobalAbilitySubsystem>(GetWorld()))
	{
		GlobalAbilitySubsystem->UnregisterASC(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UCrashAbilitySystemComponent::TryActivatePassiveAbilities()
{
	ABILITYLIST_SCOPE_LOCK();

	/* Try to activate each of this ASC's abilities as a "passive ability." TryActivatePassiveAbility will check on its
	 * own whether the ability can actually be activated this way. */
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UCrashGameplayAbilityBase* AbilityCDO = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability))
		{
			AbilityCDO->TryActivatePassiveAbility(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UCrashAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// Search for any abilities with an input tag matching the pressed tag.
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			// Queue any ability with matching input to be processed for this frame.
			if (AbilitySpec.Ability && (Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability)->GetInputTag().MatchesTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
			}
		}
	}
}

void UCrashAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// Search for any abilities with an input tag matching the released tag.
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			// Queue any ability with matching input to be processed for this frame.
			if (AbilitySpec.Ability && (Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability)->GetInputTag().MatchesTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);
			}
		}
	}
}

void UCrashAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// Check if ability input is blocked.
	if (HasMatchingGameplayTag(CrashGameplayTags::TAG_State_AbilityInputBlocked))
	{
		ClearAbilityInput();
		return;
	}


	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();


	// Process abilities that activate while their input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UCrashGameplayAbilityBase* AbilityCDO = Cast<UCrashGameplayAbilityBase>(AbilitySpec->Ability);
				if (AbilityCDO && AbilityCDO->GetActivationMethod() == EAbilityActivationMethod::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}


	// Process abilities that activate when their input is pressed.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				// If the ability is active, route the input event to it.
				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
				// If the ability is not active, try to activate it.
				else
				{
					const UCrashGameplayAbilityBase* AbilityCDO = Cast<UCrashGameplayAbilityBase>(AbilitySpec->Ability);
					if (AbilityCDO && AbilityCDO->GetActivationMethod() == EAbilityActivationMethod::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}


	// Activate all triggered abilities.
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}


	// Process abilities whose input was released.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				// If the ability was active, route the input event to it.
				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}


	// Clear cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UCrashAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UCrashAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

void UCrashAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
	}
}

bool UCrashAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup ActivationGroup) const
{
	// Independent abilities are never blocked.
	if (ActivationGroup == EAbilityActivationGroup::Independent)
	{
		return false;
	}

	// If there is no active exclusive ability, then no exclusive abilities will be blocked.
	if (!IsValid(CurrentExclusiveAbility))
	{
		return false;
	}

	// New exclusive abilities are blocked if the active exclusive ability is a blocking ability.
	return CurrentExclusiveAbility->GetActivationGroup() == EAbilityActivationGroup::Exclusive_Blocking;
}

void UCrashAbilitySystemComponent::HandleAbilityActivatedForActivationGroup(UCrashGameplayAbilityBase* ActivatedAbility)
{
	switch (ActivatedAbility->GetActivationGroup())
	{
		// We don't need to do anything for independent abilities.
		case EAbilityActivationGroup::Independent:
		{
			return;
		}

		/* If the activated ability is exclusive, cancel the active replaceable exclusive ability (if there is one) and
		 * cache the new exclusive ability. */
		case EAbilityActivationGroup::Exclusive_Blocking:
		case EAbilityActivationGroup::Exclusive_Replaceable:
		{
			if (CurrentExclusiveAbility)
			{
				/* If there is an active exclusive ability, it must be replaceable (otherwise the given ability
				 * wouldn't have been activated). */
				if (CurrentExclusiveAbility->GetActivationGroup() == EAbilityActivationGroup::Exclusive_Replaceable)
				{
					CurrentExclusiveAbility->CancelAbility(CurrentExclusiveAbility->GetCurrentAbilitySpecHandle(), CurrentExclusiveAbility->GetCurrentActorInfo(), CurrentExclusiveAbility->GetCurrentActivationInfo(), true);
				}
				/* If there is a current exclusive ability but it is not exclusive and replaceable, then it is either
				 * independent (and should not have been cached as the exclusive ability) or it is blocking (and the
				 * given ability should not have been activated). */
				else
				{
					ABILITY_LOG(Error, TEXT("UCrashAbilitySystemComponent: A replaceable ability, [%s], was activated, but the current exclusive ability was not successfully overridden."), *GetNameSafe(ActivatedAbility));
					return;
				}
			}

			// Cache the new exclusive ability.
			CurrentExclusiveAbility = ActivatedAbility;
		}
	}
}

void UCrashAbilitySystemComponent::HandleAbilityEndedForActivationGroup(UCrashGameplayAbilityBase* EndedAbility)
{
	// If the given ability is the current exclusive ability, clear it from the exclusive ability cache.
	if (CurrentExclusiveAbility == EndedAbility)
	{
		CurrentExclusiveAbility = nullptr;
	}
}

void UCrashAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// Send a standardized message communicating the ability addition.
	BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Added, AbilitySpec.Handle, AbilitySpec.Level);
}

void UCrashAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	// Send a standardized message communicating the ability removal.
	BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Removed, AbilitySpec.Handle);
}

void UCrashAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	// Send a standardized message communicating the ability activation.
	BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Activated_Success, Handle);
}

void UCrashAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	// Send a standardized message communicating the ability end.
	BroadcastAbilityMessage(CrashGameplayTags::TAG_Message_Ability_Ended, Handle);
}

void UCrashAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		// Only use CrashGameplayAbilityBase abilities in this function so the predicate can use the typed ability.
		UCrashGameplayAbilityBase* AbilityCDO = Cast<UCrashGameplayAbilityBase>(AbilitySpec.Ability);
		if (!AbilityCDO)
		{
			ABILITY_LOG(Error, TEXT("Ability [%s], not of type CrashGameplayAbilityBase, was granted to ASC [%s]. Abilities not of type CrashGameplayAbilityBase cannot be cancelled-by-function. Skipping."), *AbilitySpec.Ability.GetName(), *GetNameSafe(this));
			continue;
		}

		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Instanced abilities: cancel all the instances of the ability (not the CDO).
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
			for (UGameplayAbility* AbilityInstance : Instances)
			{
				UCrashGameplayAbilityBase* CrashAbilityInstance = CastChecked<UCrashGameplayAbilityBase>(AbilityInstance);

				if (ShouldCancelFunc(CrashAbilityInstance, AbilitySpec.Handle))
				{
					if (CrashAbilityInstance->CanBeCanceled())
					{
						CrashAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), CrashAbilityInstance->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						ABILITY_LOG(Error, TEXT("CancelAbilitiesByFunc failed to cancel ability [%s]: CanBeCanceled is false."), *GetNameSafe(CrashAbilityInstance));
					}
				}
			}
		}
		else
		{
			// Non-instanced abilities: cancel the CDO.
			if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
	}
}

void UCrashAbilitySystemComponent::DisableAbilitiesByTag(const FGameplayTagContainer& Tags)
{
	if (!IsOwnerActorAuthoritative())
	{
		ABILITY_LOG(Error, TEXT("DisableAbilitiesByTag called on the client. This method can only be called with authority."));
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	// Search for abilities with any matching tags.
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasAny(Tags))
		{
			// Disable the ability.
			AbilitySpec.Ability->AbilityTags.AddTag(CrashGameplayTags::TAG_Ability_Behavior_Disabled);
		}
	}
}

void UCrashAbilitySystemComponent::EnableAbilitiesByTag(const FGameplayTagContainer& Tags)
{
	if (!IsOwnerActorAuthoritative())
	{
		ABILITY_LOG(Error, TEXT("EnableAbilitiesByTag called on the client. This method can only be called with authority."));
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	// Search for abilities with any matching tags.
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.Ability->AbilityTags.HasAny(Tags))
		{
			// Only try to re-enable abilities that are disabled.
			if (AbilitySpec.Ability->AbilityTags.HasTagExact(CrashGameplayTags::TAG_Ability_Behavior_Disabled))
			{
				// Enable the ability.
				AbilitySpec.Ability->AbilityTags.RemoveTag(CrashGameplayTags::TAG_Ability_Behavior_Disabled);
			}
		}
	}
}

void UCrashAbilitySystemComponent::SetCurrentKnockbackSource(AActor* Source)
{
	if (ensure(IsValid(Source)))
	{
		CurrentKnockbackSource = Source;
	}
}

void UCrashAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	// Handle execution of the gameplay cue directly, without replication.
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters, EGameplayCueExecutionOptions::IgnoreSuppression);
}

void UCrashAbilitySystemComponent::AddGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	// Handle addition of the gameplay cue locally.
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters, EGameplayCueExecutionOptions::IgnoreSuppression);
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters, EGameplayCueExecutionOptions::IgnoreSuppression);
}

void UCrashAbilitySystemComponent::RemoveGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	// Handle removal of the gameplay cue locally.
	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters, EGameplayCueExecutionOptions::IgnoreSuppression);
}

float UCrashAbilitySystemComponent::PlayMontage_FirstPerson(UGameplayAbility* InAnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName, float StartTimeSeconds)
{
	float Duration = -1.0f;

	// Get the first-person mesh, if the avatar is a CrashCharacter.
	ACrashCharacter* CrashCharacter = GetAvatarActor() ? Cast<ACrashCharacter>(GetAvatarActor()) : nullptr;

	// First-person montages cannot be played without a CrashCharacter avatar. No other actors have a first-person mesh.
	if (!CrashCharacter)
	{
		return Duration;
	}

	// Retrieve the first-person mesh animation instance.
	// USkeletalMeshComponent* FirstPersonMesh = CrashCharacter->GetFirstPersonMesh();
	// UAnimInstance* AnimInstance = FirstPersonMesh ? FirstPersonMesh->GetAnimInstance() : nullptr;
	UAnimInstance* AnimInstance = AbilityActorInfo.IsValid() ? GetCrashAbilityActorInfo()->GetFirstPersonAnimInstance() : nullptr;

	if (AnimInstance && Montage)
	{
		// Play the montage locally. This is predicted on the owning client.
		Duration = AnimInstance->Montage_Play(Montage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);

		if (Duration > 0.f)
		{
			UAnimSequenceBase* Animation = Montage->IsDynamicMontage() ? Montage->GetFirstAnimReference() : Montage;

			if (Montage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UCrashAbilitySystemComponent::PlayMontage_FirstPerson [%s], Role: [%s]")
					, *GetNameSafe(Montage)
					, *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole())
					);
			}

			// Start the montage at the given section, if desired.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
			}

			/* Replicate for non-owners and for replay recordings. The data we set from GetRepAnimMontageInfo_Mutable()
			 * is used both by the server to replicate to clients and by clients to record replays. We need to set this
			 * data for recording clients because there exists network configurations where an ability's montage data
			 * will not replicate to some clients (e.g. if the client is an autonomous proxy). */
			if (ShouldRecordMontageReplication())
			{
				FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo = GetRepAnimMontageInfo_Mutable();

				// Static parameters. These are set when the montage is played and are never changed after.
				MutableRepAnimMontageInfo.Animation = Animation;
				MutableRepAnimMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0);

				MutableRepAnimMontageInfo.SectionIdToPlay = 0;
				if (MutableRepAnimMontageInfo.Animation && StartSectionName != NAME_None)
				{
					// Add one so INDEX_NONE can be used in the OnRep.
					MutableRepAnimMontageInfo.SectionIdToPlay = Montage->GetSectionIndex(StartSectionName) + 1;
				}

				// Update replicated data that changed during the montage's lifetime.
				AnimMontage_UpdateReplicatedData();
			}

			// Replicate to non-owners.
			if (IsOwnerActorAuthoritative())
			{
				// Force net update on the avatar actor.
				if (AbilityActorInfo->AvatarActor != nullptr)
				{
					AbilityActorInfo->AvatarActor->ForceNetUpdate();
				}
			}
			// Listen for potential prediction rejection.
			else
			{
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					// If this prediction key is rejected, we need to stop the predicted montage.
					PredictionKey.NewRejectedDelegate().BindUObject(this, &UCrashAbilitySystemComponent::OnFirstPersonPredictiveMontageRejected, Montage);
				}
			}
		}
	}

	return Duration;
}

void UCrashAbilitySystemComponent::OnFirstPersonPredictiveMontageRejected(UAnimMontage* PredictiveMontage)
{
	static const float MONTAGE_PREDICTION_REJECT_FADETIME = 0.25f;

	// Get the first-person mesh, if the avatar is a CrashCharacter.
	UAnimInstance* AnimInstance = nullptr;
	ACrashCharacter* CrashCharacter = GetAvatarActor() ? Cast<ACrashCharacter>(GetAvatarActor()) : nullptr;

	/* First-person montages cannot be played without a CrashCharacter avatar. This should never happen, since we were
	 * able to play the montage. */
	if (!CrashCharacter)
	{
		ABILITY_LOG(Error, TEXT("Attempted to reject predicted first-person montage [%s] without a valid CrashCharacter avatar. Avatar may have been lost."), *GetNameSafe(PredictiveMontage));
		return;
	}

	// Retrieve the first-person mesh animation instance.
	USkeletalMeshComponent* FirstPersonMesh = CrashCharacter->GetFirstPersonMesh();
	AnimInstance = FirstPersonMesh ? FirstPersonMesh->GetAnimInstance() : nullptr;

	if (AnimInstance && PredictiveMontage)
	{
		// If this montage is still playing, stop it.
		if (AnimInstance->Montage_IsPlaying(PredictiveMontage))
		{
			AnimInstance->Montage_Stop(MONTAGE_PREDICTION_REJECT_FADETIME, PredictiveMontage);
		}
	}
}

const FCrashGameplayAbilityActorInfo* UCrashAbilitySystemComponent::GetCrashAbilityActorInfo() const
{
	// Cast to typed actor info.
	return static_cast<const FCrashGameplayAbilityActorInfo*>(AbilityActorInfo.Get());
}

FGameplayEffectContextHandle UCrashAbilitySystemComponent::MakeEffectContextWithHitResult(const FHitResult& HitResult) const
{
	FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());

	// By default use the owner and avatar as the instigator and causer
	if (ensureMsgf(AbilityActorInfo.IsValid(), TEXT("Unable to make effect context because AbilityActorInfo is not valid.")))
	{
		Context.AddInstigator(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get());
	}

	Context.AddHitResult(HitResult);

	return Context;
}

void UCrashAbilitySystemComponent::BroadcastAbilityMessage(const FGameplayTag MessageType, const FGameplayAbilitySpecHandle& Ability, const float Magnitude, bool bReplicateMessage)
{
	ensure(MessageType.IsValid());
	if (!UGameplayMessageSubsystem::HasInstance(GetWorld()))
	{
		return;
	}

	// Construct the message.
	FCrashAbilityMessage Message;
	Message.MessageType = MessageType;
	Message.AbilitySpecHandle = Ability;
	Message.ActorInfo = *GetCrashAbilityActorInfo();
	Message.Magnitude = Magnitude;

	// Broadcast the message locally.
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSystem.BroadcastMessage(Message.MessageType, Message);

	// Some events only occur on the server. If we should replicate this message to clients, use a reliable multicast.
	if (bReplicateMessage && AbilityActorInfo->IsNetAuthority())
	{
		MulticastReliableAbilityMessageToClients(Message);
	}
}

void UCrashAbilitySystemComponent::MulticastReliableAbilityMessageToClients_Implementation(const FCrashAbilityMessage Message)
{
	// Locally broadcast the received message if this is a client.
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.MessageType, Message);
	}
}
