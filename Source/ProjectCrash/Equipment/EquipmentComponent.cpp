// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EquipmentAnimationData.h"
#include "EquipmentLog.h"
#include "EquipmentSet.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Characters/ChallengerBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"

UEquipmentComponent::UEquipmentComponent()
{
}

void UEquipmentComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that this component is in a character class.
	const ACharacter* Char = GetOwner<ACharacter>();
	ensureAlwaysMsgf((Char != nullptr), TEXT("EquipmentComponent on [%s] can only be added to character actors."), *GetNameSafe(GetOwner()));

	// Ensure that this is the only instance of this component in its owner.
	TArray<UActorComponent*> ASCExtensionComponents;
	Char->GetComponents(UEquipmentComponent::StaticClass(), ASCExtensionComponents);
	ensureAlwaysMsgf((ASCExtensionComponents.Num() == 1), TEXT("Only one EquipmentComponent should exist on [%s]."), *GetNameSafe(GetOwner()));
}

void UEquipmentComponent::InitializeComponent()
{
	Super::InitializeComponent();

	/* Bind the OnTemporarilyUnequippedChanged callback to when this component's owner's ASC gains or loses the
	 * TemporarilyUnequipped tag. */
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		TemporarilyUnequippedDelegate = ASC->RegisterGameplayTagEvent(CrashGameplayTags::TAG_State_TemporarilyUnequipped, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UEquipmentComponent::OnTemporarilyUnequippedChanged);
	}
}

UEquipmentSet* UEquipmentComponent::EquipEquipmentSet(UEquipmentSet* SetToEquip)
{
	// Unequip the currently equipped equipment set, if there is one.
	UEquipmentSet* PreviousSet = CurrentEquipmentSet;
	if (PreviousSet)
	{
		if (UnequipSet_Internal())
		{
			CurrentEquipmentSet = nullptr;
		}
	}

	// Equip the new equipment set.
	if (SetToEquip)
	{
		if (EquipSet_Internal(SetToEquip))
		{
			CurrentEquipmentSet = SetToEquip;
		}
	}

	return PreviousSet ? PreviousSet : nullptr;
}

bool UEquipmentComponent::EquipSet_Internal(UEquipmentSet* SetToEquip)
{
	check(SetToEquip);

	/* We cannot equip a new equipment set if we already have on equipped. EquipEquipmentSet handles this for us, which
	 * is why we don't call this function directly. */
	if (CurrentEquipmentSet)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to equip equipment set [%s] on actor [%s], but the actor's current equipment set, [%s], must be unequipped first."), *SetToEquip->GetName(), *GetNameSafe(GetOwner()), *CurrentEquipmentSet->GetName());
		return false;
	}

	// Define parameters for attaching equipment actors.
	ACharacter* EquippingChar = Cast<ACharacter>(GetOwner());

	// Define parameters for spawning the equipment actors.
	FActorSpawnParameters SpawnParams = FActorSpawnParameters();
	SpawnParams.Owner = EquippingChar->GetPlayerState();
	SpawnParams.Instigator = EquippingChar;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform SpawnTransform = FTransform();
	const FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	
	// Spawn each first-person equipment actor.
	for (FEquipmentActorData EquipmentActor : SetToEquip->EquipmentActors)
	{
		// Only playable characters need to spawn separate actors for first-person.
		if (IsValid(EquipmentActor.SpawnedActorClasses.ActorToSpawn_FPP) && EquippingChar->IsA(AChallengerBase::StaticClass()))
		{
			// TODO: Create a handle for the new equipment set.

			// Spawn the first-person actor.
			EquipmentActor.SpawnedActor_FPP = GetWorld()->SpawnActor(EquipmentActor.SpawnedActorClasses.ActorToSpawn_FPP, &SpawnTransform, SpawnParams);

			// Attach the first-person actor to the specified socket.
			EquipmentActor.SpawnedActor_FPP->AttachToComponent(Cast<AChallengerBase>(EquippingChar)->GetFirstPersonMesh(), AttachRules, EquipmentActor.AttachSockets.AttachSocket_FPP);
			EquipmentActor.SpawnedActor_FPP->SetActorRelativeTransform(EquipmentActor.AttachOffsets.AttachOffset_FPP);
		}
	}

	// Spawn each third-person equipment actor.
	for (FEquipmentActorData EquipmentActor : SetToEquip->EquipmentActors)
	{
		if (IsValid(EquipmentActor.SpawnedActorClasses.ActorToSpawn_TPP))
		{
			// Spawn the third-person actor.
			EquipmentActor.SpawnedActor_TPP = GetWorld()->SpawnActor(EquipmentActor.SpawnedActorClasses.ActorToSpawn_TPP, &SpawnTransform, SpawnParams);

			// Attach the third-person actor to the specified socket.
			USceneComponent* ParentComp = Cast<AChallengerBase>(EquippingChar) ? Cast<AChallengerBase>(EquippingChar)->GetThirdPersonMesh() : EquippingChar->GetMesh();
			EquipmentActor.SpawnedActor_TPP->AttachToComponent(ParentComp, AttachRules, EquipmentActor.AttachSockets.AttachSocket_TPP);
			EquipmentActor.SpawnedActor_TPP->SetActorRelativeTransform(EquipmentActor.AttachOffsets.AttachOffset_TPP);
		}
	}

	// Grant the equipment's ability set to the equipping character.
	if (SetToEquip->GrantedAbilitySet)
	{
		if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
		{
			// Only grant ability sets on the server.
			if (CrashASC->IsOwnerActorAuthoritative())
			{
				SetToEquip->GrantedAbilitySet->GiveToAbilitySystem(CrashASC, &SetToEquip->GrantedAbilitySetHandles, SetToEquip);
			}
		}
	}

	// Update the animation instance being used by the character meshes.
	if (const UEquipmentAnimationData* AnimData = SetToEquip->AnimationData)
	{
		if (const TSubclassOf<UAnimInstance> EquipmentAnimInstance = AnimData->EquipmentAnimInstance)
		{
			if (const AChallengerBase* CharAsChallenger = Cast<AChallengerBase>(EquippingChar))
			{
				// Cache mesh components.
				USkeletalMeshComponent* FPPMesh = CharAsChallenger->GetFirstPersonMesh();
				USkeletalMeshComponent* TPPMesh = CharAsChallenger->GetThirdPersonMesh();

				// Update the animation instance.
				FPPMesh->SetAnimInstanceClass(EquipmentAnimInstance);
				TPPMesh->SetAnimInstanceClass(EquipmentAnimInstance);

				// Play the first-person "equip" animation.
				if (UAnimMontage* EquipFPP = AnimData->EquipAnimation.Anim_FPP)
				{
					FPPMesh->GetAnimInstance()->Montage_Play(EquipFPP);
				}

				// Play the third-person "equip" animation.
				if (UAnimMontage* EquipTPP = AnimData->EquipAnimation.Anim_TPP)
				{
					TPPMesh->GetAnimInstance()->Montage_Play(EquipTPP);
				}
			}
			// Update the TPP animation instance and play the third-person "equip" animation for non-player characters.
			else
			{
				EquippingChar->GetMesh()->SetAnimInstanceClass(EquipmentAnimInstance);

				if (IsValid(AnimData->EquipAnimation.Anim_TPP))
				{
					EquippingChar->GetMesh()->GetAnimInstance()->Montage_Play(AnimData->EquipAnimation.Anim_TPP);
				}
			}
		}
	}

	return true;
}

bool UEquipmentComponent::UnequipSet_Internal()
{
	/* If we don't have an equipment set equipped, we don't need to do anything. */
	if (!CurrentEquipmentSet)
	{
		return false;
	}

	// Destroy each equipment actor.
	for (FEquipmentActorData EquipmentActor : CurrentEquipmentSet->EquipmentActors)
	{
		if (IsValid(EquipmentActor.SpawnedActor_FPP))
		{
			EquipmentActor.SpawnedActor_FPP->Destroy();
		}

		if (IsValid(EquipmentActor.SpawnedActor_TPP))
		{
			EquipmentActor.SpawnedActor_TPP->Destroy();
		}
	}

	// Remove the ability set that was granted by the equipped equipment set (if there was one).
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		CurrentEquipmentSet->GrantedAbilitySetHandles.RemoveFromAbilitySystem(CrashASC);
	}

	return true;
}

void UEquipmentComponent::OnTemporarilyUnequippedChanged(const FGameplayTag Tag, int32 NewCount)
{
	/* Temporarily unequip this character's current equipment set when the TemporarilyUnequipped tag is gained. We
	 * maintain a reference to the equipment set with CurrentEquipmentSet. */
	if (NewCount > 0)
	{
		UnequipSet_Internal();
	}
	/* Re-equip this character's current equipment set when all TemporarilyUnequipped tags have been removed. If our
	 * equipment set changed while we were unequipped, the new set will be equipped instead.
	 */
	else
	{
		if (CurrentEquipmentSet)
		{
			EquipSet_Internal(CurrentEquipmentSet);
		}
	}
}
