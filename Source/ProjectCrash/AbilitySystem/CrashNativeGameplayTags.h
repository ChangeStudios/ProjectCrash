// Copyright Samuel Reitich 2024.

#pragma once

#include "GameplayTagContainer.h"

class UGameplayTagsManager;

/**
 * Singleton containing gameplay tags that need to be exposed to C++. All tags that are exposed to C++ should be
 * defined here.
 */
struct FCrashNativeGameplayTags
{

	// Class management.

private:

	/** Singleton instance of this struct. */
	static FCrashNativeGameplayTags GameplayTags;

public:

	/** CrashNativeGameplayTags singleton getter. */
	static const FCrashNativeGameplayTags& Get() { return GameplayTags; }



	// Tag management.

public:

	/** Loads the native tags into the framework. This MUST be called on game initialization to properly
	 * use these tags. */
	static void InitializeNativeTags();

protected:

	/** Defines and adds the native gameplay tags. This is where we define the gameplay tags for the tag manager. */
	void AddAllTags(UGameplayTagsManager& Manager);

private:

	/** Adds the given gameplay tag to the specified gameplay tag manager. Helper function for AddAllTags. */
	void AddTag(UGameplayTagsManager& Manager, FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagDescription);



	// Gameplay tags. Tags can be referenced in C++ using these handles.

public:

	// Abilities
	FGameplayTag TAG_Ability_Behavior_PersistsThroughAvatarDestruction;

	FGameplayTag TAG_Ability_Identifier_Generic_Jump;


	// Effects
	FGameplayTag TAG_Effects_Damage_SelfDestruct;
	FGameplayTag TAG_Effects_Damage_CanDamageSelf;
	FGameplayTag TAG_Effects_Damage_CanDamageTeam;


	// Events
	FGameplayTag TAG_Event_Death;

	
	// Input
	FGameplayTag TAG_InputTag_Jump;
	FGameplayTag TAG_InputTag_Look_Mouse;
	FGameplayTag TAG_InputTag_Look_Stick;
	FGameplayTag TAG_InputTag_Move;


	// States
	FGameplayTag TAG_State_ImmuneToDamage;
};
