#include "CrashNativeGameplayTags.h"

#include "GameplayTagsManager.h"

FCrashNativeGameplayTags FCrashNativeGameplayTags::GameplayTags;

void FCrashNativeGameplayTags::InitializeNativeTags()
{
	// Get the gameplay tag manager.
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// Add every native gameplay tag to the gameplay tag manager.
	GameplayTags.AddAllTags(Manager);

	// We have to notify the gameplay tag manager that we are done adding native gameplay tags.
	Manager.DoneAddingNativeTags();
}

void FCrashNativeGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	// Define tags here.
	AddTag(Manager, TAG_Ability_Behavior_PersistsThroughAvatarDestruction, "Ability.Behavior.PersistsThroughAvatarDestruction", "Indicates that an ability is not canceled when its ASC’s avatar dies or is unpossessed.");

	AddTag(Manager, TAG_Ability_Identifier_Generic_Jump, "Ability.Identifier.Generic.Jump", "Identifier for the generic character jump ability. Includes abilities like double-jump.");


	AddTag(Manager, TAG_Effects_Damage_SelfDestruct, "Effects.Damage.SelfDestruct", "Self-destruct damage. This overrides any damage invulnerabilities.");
	AddTag(Manager, TAG_Effects_Damage_CanDamageSelf, "Effects.Damage.CanDamageSelf", "Specifies that damage caused by this gameplay effect can affect the source actor.");
	AddTag(Manager, TAG_Effects_Damage_CanDamageTeam, "Effects.Damage.CanDamageTeam", "Specifies that damage caused by this gameplay effect can affect the source actor's team. Does not enable damage on the source actor themselves.");


	AddTag(Manager, TAG_Event_Death, "Event.Death", "Event triggered when this ASC’s avatar \"dies.\"");


	AddTag(Manager, TAG_InputTag_Jump, "InputTag.Jump", "Input tag to activate the jump ability.");
	AddTag(Manager, TAG_InputTag_Look_Mouse, "InputTag.Look.Mouse", "Input tag to activate the look action with a mouse.");
	AddTag(Manager, TAG_InputTag_Look_Stick, "InputTag.Look.Stick", "Input tag to activate the look action with a gamepad.");
	AddTag(Manager, TAG_InputTag_Move, "InputTag.Move", "Input tag to activate the move action.");


	AddTag(Manager, TAG_State_ImmuneToDamage, "State.ImmuneToDamage", "The target is currently immune to all incoming damage. Can be overridden by effects with the SelfDestruct tag.");
}

void FCrashNativeGameplayTags::AddTag(UGameplayTagsManager& Manager, FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagDescription)
{
	/* Construct a new gameplay tag with the given data and pass it to the specified gameplay tag manager. "Native"
	 * specifies that this structure is the source of the tag. */
	OutTag = Manager.AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagDescription));
}
