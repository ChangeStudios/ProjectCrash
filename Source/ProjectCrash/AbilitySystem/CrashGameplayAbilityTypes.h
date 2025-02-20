// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetDataFilter.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "CrashGameplayTags.h"
#include "GameFramework/Teams/TeamSubsystem.h"
#include "CrashGameplayAbilityTypes.generated.h"

class ACrashPlayerState;
class ACrashPlayerController;
class ACrashCharacter;
class UCrashAbilitySystemComponent;

/**
 * Actor info structure for this project. Adds properites and accessors for project-specific actors, and adds separate
 * first-person and third-person skeletal mesh component and animation instance properties for avatars of the
 * CrashCharacter type.
 *
 * Note that this struct's default SkeletalMeshComponent and AnimInstance are the third-person mesh and animation
 * instance. This struct only adds first-person properties, but also ensures the correct data is cached for the
 * third-person properties.
 */
USTRUCT(BlueprintType)
struct PROJECTCRASH_API FCrashGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	// Construction.

public:

	/** Default constructor. */
	FCrashGameplayAbilityActorInfo() {}



	// Initialization.

public:

	/** Caches as much actor information as possible. It's rare that we fill out this entire struct. */
	virtual void InitFromActor(AActor* OwnerActor, AActor* AvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;

	// Clears cached actor info.
	virtual void ClearActorInfo() override;



	// Custom properties.

// Project-specific actors.
public:

	/** The ability's ASC, cached as a CrashAbilitySystemComponent. Should never be null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<UCrashAbilitySystemComponent> CrashAbilitySystemComponent;

	/** The player controller associated with the owning actor, cached as a CrashPlayerController. Often null. Always
	 * null on simulated proxies. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashPlayerController> CrashPlayerController;

	/** The player state for the controller associated with the owning actor, if it has one. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashPlayerState> CrashPlayerState;

	/** The avatar as a CrashCharacter. Often null. Always null if the avatar is not of type CrashCharacter. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<ACrashCharacter> CrashCharacter;

	/** First-person mesh of the avatar actor, if the avatar is of type CrashCharacter. Often null. */
	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<USkeletalMeshComponent> FirstPersonSkeletalMeshComponent;

// Accessors.
public:

	/** Returns the actor info's ability system component as a CrashAbilitySystemComponent. Should never be null. */
	UCrashAbilitySystemComponent* GetCrashAbilitySystemComponent() const;

	/** Returns the actor info's current player controller, as a CrashPlayerController. Often null. Always null on
	 * simulated proxies. */
	ACrashPlayerController* GetCrashPlayerController() const;

	/** Returns the actor info's current player state, as a CrashPlayerState. Often null. */
	ACrashPlayerState* GetCrashPlayerState() const;

	/** Returns the affected animation instance from the FirstPersonSkeletalMeshComponent. Null for avatars that are not
	 * of type CrashCharacter. */
	UAnimInstance* GetFirstPersonAnimInstance() const;
};



/** Which targets to filter out depending on their team alignment to self. */
UENUM(BlueprintType)
namespace ETargetDataFilterTeam
{
	enum Type : int
	{
		TDFT_Any 			UMETA(DisplayName = "Allow any team"),
		// Filters out any targets with the same team as self. Does not filter self out.
		TDFT_NoTeammates 	UMETA(DisplayName = "Filter teammates out"),
		// Filters out any targets with a different team from self, a neutral team, or no team.
		TDFT_NoEnemies		UMETA(DisplayName = "Filter enemies out")
	};
}

/**
 * A target data filter that can filter targets depending on their team alignment. Also automatically filters out any
 * actors without an ability system component.
 */
USTRUCT(BlueprintType)
struct FCrashTargetDataFilter : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

	/** Returns true if the actor has an associated ability system component and passes both filters. */
	virtual bool FilterPassesForActor(const AActor* ActorToBeFiltered) const override
	{
		// Self filter.
		if (!Super::FilterPassesForActor(ActorToBeFiltered))
		{
			return (bReverseFilter ^ false);
		}

		// Team filter.
		if (IsValid(ActorToBeFiltered) && IsValid(SelfActor))
		{
			UTeamSubsystem* TeamSubsystem = ActorToBeFiltered->GetWorld()->GetSubsystem<UTeamSubsystem>();
			if (ensure(TeamSubsystem))
			{
				ETeamAlignment Alignment = TeamSubsystem->CompareTeams(ActorToBeFiltered, SelfActor);

				switch (TeamFilter)
				{
				// Filter out enemies and neutral agents.
				case ETargetDataFilterTeam::Type::TDFT_NoEnemies:
					if (Alignment == ETeamAlignment::DifferentTeams ||
						Alignment == ETeamAlignment::InvalidArgument)
					{
						return (bReverseFilter ^ false);
					}
					break;
				// Filter out teammates.
				case ETargetDataFilterTeam::Type::TDFT_NoTeammates:
					if (Alignment == ETeamAlignment::SameTeam)
					{
						// Don't filter self out; the self filter will handle that.
						if (ActorToBeFiltered != SelfActor)
						{
							return (bReverseFilter ^ false);
						}
					}
					break;
				case ETargetDataFilterTeam::Type::TDFT_Any:
				default:
					break;
				}
			}
		}

		// All targets must have an ability system component.
		UAbilitySystemComponent* ActorASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ActorToBeFiltered);
		if (ActorASC == nullptr)
		{
			return (bReverseFilter ^ false);
		}
		// Invulnerability filter.
		else if (ActorASC->HasMatchingGameplayTag(CrashGameplayTags::TAG_State_Invulnerable))
		{
			if (bIgnoreInvulnerableTargets)
			{
				return (bReverseFilter ^ false);
			}
		}

		return (bReverseFilter ^ true);
	}

	/** Filter based on the team alignment of the target, relative to Self actor. Self must be set. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category = Filter)
	TEnumAsByte<ETargetDataFilterTeam::Type> TeamFilter = ETargetDataFilterTeam::TDFT_Any;

	/** If true, targets with the "Invulnerable" status effect will be filtered out. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category = Filter)
	bool bIgnoreInvulnerableTargets = true;
};
