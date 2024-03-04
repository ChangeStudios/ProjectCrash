// Copyright Samuel Reitich 2024.


#include "Equipment/EquipmentComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EquipmentAnimationData.h"
#include "EquipmentSet.h"
#include "AbilitySystem/CrashAbilitySystemGlobals.h"
#include "AbilitySystem/CrashGameplayTags.h"
#include "AbilitySystem/Components/CrashAbilitySystemComponent.h"
#include "Animation/ChallengerAnimInstanceBase.h"
#include "Characters/ChallengerBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Characters/ChallengerData.h"
#include "GameFramework/CrashLogging.h"

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

	if (!GetOwner() || !GetWorld())
	{
		return false;
	}

	/* We cannot equip a new equipment set if we already have on equipped. EquipEquipmentSet handles this for us, which
	 * is why we don't call this function directly. We check the equipment set handle instead of the equipment set
	 * itself because we may still need to equip an equipment set even if the latter is still valid; i.e. re-equipping
	 * a temporarily unequipped set. */
	if (CurrentEquipmentSetHandle.EquipmentSet)
	{
		EQUIPMENT_LOG(Warning, TEXT("Attempted to equip equipment set [%s] on actor [%s], but the actor's current equipment set, [%s], must be unequipped first."), *SetToEquip->GetName(), *GetNameSafe(GetOwner()), *CurrentEquipmentSet->GetName());
		return false;
	}
	else
	{
		EQUIPMENT_LOG(Verbose, TEXT("Equipped [%s] on [%s]."), *SetToEquip->GetName(), *GetNameSafe(GetOwner()));
	}

	// Cache the equipment set class being equipped.
	CurrentEquipmentSetHandle.EquipmentSet = SetToEquip;

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
	for (FEquipmentActorData EquipmentActor : SetToEquip->FirstPersonEquipmentActors)
	{
		// Only playable characters need to spawn separate actors for first-person.
		if (EquipmentActor.SpawnedActorClass && EquippingChar->IsA(AChallengerBase::StaticClass()))
		{
			// Spawn the first-person actor.
			AActor* SpawnedActor_FPP = GetWorld()->SpawnActor(EquipmentActor.SpawnedActorClass, &SpawnTransform, SpawnParams);
			SpawnedActor_FPP->SetOwner(GetOwner());

			// Attach the first-person actor to the specified socket.
			SpawnedActor_FPP->AttachToComponent(Cast<AChallengerBase>(EquippingChar)->GetFirstPersonMesh(), AttachRules, EquipmentActor.AttachSocket);
			SpawnedActor_FPP->SetActorRelativeTransform(EquipmentActor.AttachOffset);

			// Cache the new actor in the current equipment set handle.
			CurrentEquipmentSetHandle.SpawnedActors_FPP.Add(SpawnedActor_FPP);
		}
	}

	// Spawn each third-person equipment actor.
	for (FEquipmentActorData EquipmentActor : SetToEquip->ThirdPersonEquipmentActors)
	{
		if (IsValid(EquipmentActor.SpawnedActorClass))
		{
			// Spawn the third-person actor.
			AActor* SpawnedActor_TPP = GetWorld()->SpawnActor(EquipmentActor.SpawnedActorClass, &SpawnTransform, SpawnParams);
			SpawnedActor_TPP->SetOwner(GetOwner());

			// Attach the third-person actor to the specified socket.
			USceneComponent* ParentComp = Cast<AChallengerBase>(EquippingChar) ? Cast<AChallengerBase>(EquippingChar)->GetThirdPersonMesh() : EquippingChar->GetMesh();
			SpawnedActor_TPP->AttachToComponent(ParentComp, AttachRules, EquipmentActor.AttachSocket);
			SpawnedActor_TPP->SetActorRelativeTransform(EquipmentActor.AttachOffset);

			// Cache the new actor in the current equipment set handle.
			CurrentEquipmentSetHandle.SpawnedActors_TPP.Add(SpawnedActor_TPP);
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
				SetToEquip->GrantedAbilitySet->GiveToAbilitySystem(CrashASC, &CurrentEquipmentSetHandle.GrantedAbilitySetHandles, SetToEquip);
			}
		}
	}

	// Update the animation instance being used by the character meshes.
	if (const UEquipmentAnimationData* AnimData = SetToEquip->AnimationData)
	{
		if (const ACrashCharacterBase* CrashChar = Cast<ACrashCharacterBase>(EquippingChar))
		{
			// Update the mesh's animation instances.
			USkeletalMeshComponent* FPPMesh = CrashChar->GetFirstPersonMesh();
			if (UChallengerAnimInstanceBase* FPPAnimInstance = Cast<UChallengerAnimInstanceBase>(FPPMesh->GetAnimInstance()))
			{
				FPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData);
			}

			USkeletalMeshComponent* TPPMesh = CrashChar->GetThirdPersonMesh();
			if (UChallengerAnimInstanceBase* TPPAnimInstance = Cast<UChallengerAnimInstanceBase>(TPPMesh->GetAnimInstance()))
			{
				TPPAnimInstance->UpdateAnimData(SetToEquip->AnimationData);
			}

			// Play the first-person "equip" animation.
			if (UAnimMontage* EquipFPP = SetToEquip->AnimationData->Equip_FPP)
			{
				FPPMesh->GetAnimInstance()->Montage_Play(EquipFPP);
			}

			// Play the third-person "equip" animation.
			if (UAnimMontage* EquipTPP = SetToEquip->AnimationData->Equip_TPP)
			{
				TPPMesh->GetAnimInstance()->Montage_Play(EquipTPP);
			}
		}
		// Update the TPP animation instance and play the third-person "equip" animation for non-Crash characters.
		else
		{
			if (UChallengerAnimInstanceBase* CharAnimInstance = Cast<UChallengerAnimInstanceBase>(EquippingChar->GetMesh()->GetAnimInstance()))
			{
				CharAnimInstance->UpdateAnimData(SetToEquip->AnimationData);
			}

			if (IsValid(AnimData->Equip_TPP))
			{
				EquippingChar->GetMesh()->GetAnimInstance()->Montage_Play(AnimData->Equip_TPP);
			}
		}
	}
	/* If we aren't given equipment animation data, fall back to the character's default animation data. This only
	 * works for Challenger characters. */ 
	else
	{
		EQUIPMENT_LOG(Warning, TEXT("[%s] equipped a new equipment set: [%s], but could not find valid equipment animation data. Falling back to default character data."), *GetNameSafe(GetOwner()), *GetNameSafe(SetToEquip));

		if (const AChallengerBase* CharAsChallenger = Cast<AChallengerBase>(EquippingChar))
		{
			if (CharAsChallenger->GetChallengerData() && CharAsChallenger->GetChallengerData()->DefaultAnimData)
			{
				// Update the first-person mesh's animation data.
				if (UChallengerAnimInstanceBase* FPPAnimInstance = Cast<UChallengerAnimInstanceBase>(CharAsChallenger->GetFirstPersonMesh()->GetAnimInstance()))
				{
					FPPAnimInstance->UpdateAnimData(CharAsChallenger->GetChallengerData()->DefaultAnimData);
				}

				// Update the third-person mesh's animation data.
				if (UChallengerAnimInstanceBase* TPPAnimInstance = Cast<UChallengerAnimInstanceBase>(CharAsChallenger->GetThirdPersonMesh()->GetAnimInstance()))
				{
					TPPAnimInstance->UpdateAnimData(CharAsChallenger->GetChallengerData()->DefaultAnimData);
				}
			}
		}
	}

	return true;
}

bool UEquipmentComponent::UnequipSet_Internal()
{
	// If we don't have an equipment set equipped, we don't need to do anything.
	if (!CurrentEquipmentSet)
	{
		return false;
	}

	// Destroy each equipment actor.
	TArray<AActor*> EquipmentActors = CurrentEquipmentSetHandle.SpawnedActors_FPP;
	EquipmentActors.Append(CurrentEquipmentSetHandle.SpawnedActors_TPP);
	for (AActor* EquipmentActor : EquipmentActors)
	{
		EquipmentActor->Destroy();
	}

	// Clear the equipment set handle's spawned actors.
	CurrentEquipmentSetHandle.SpawnedActors_FPP.Empty();
	CurrentEquipmentSetHandle.SpawnedActors_TPP.Empty();

	// Remove the ability set that was granted by the equipped equipment set (if there was one).
	if (UCrashAbilitySystemComponent* CrashASC = UCrashAbilitySystemGlobals::GetCrashAbilitySystemComponentFromActor(GetOwner()))
	{
		CurrentEquipmentSetHandle.GrantedAbilitySetHandles.RemoveFromAbilitySystem(CrashASC);
		CurrentEquipmentSetHandle.GrantedAbilitySetHandles = FCrashAbilitySet_GrantedHandles();
	}

	// Null the current equipment set's class.
	CurrentEquipmentSetHandle.EquipmentSet = nullptr;

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
