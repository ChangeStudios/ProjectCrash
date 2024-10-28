// Copyright Samuel Reitich. All rights reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/CrashAttributeSet.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Messages/CrashVerbMessage.h"
#include "LivesAttributeSet.generated.h"

/**
 * Handles players' "lives." A player loses a life when they die. When a player loses all of their lives, they can no
 * longer respawn, and usually enter a spectating state. By default, lives are shared by all players on a team.
 *
 * Lives are not used in all game modes. In most game modes that use lives, players lose the game upon running out.
 */
UCLASS(BlueprintType)
class PROJECTCRASH_API ULivesAttributeSet : public UCrashAttributeSet
{
	GENERATED_BODY()

public:

	ULivesAttributeSet();

	virtual void PostInitProperties() override;

protected:

	/** Clamps */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

private:

	FGameplayMessageListenerHandle DeathListener;

	UFUNCTION()
	void OnPlayerDeath(FGameplayTag Channel, const FCrashVerbMessage& Message);

public:

	ATTRIBUTE_ACCESSORS(ULivesAttributeSet, Lives);

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Lives, Category = "Ability|Attribute|GameMode", Meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Lives;

protected:

	UFUNCTION()
	void OnRep_Lives(const FGameplayAttributeData& OldValue);
};
