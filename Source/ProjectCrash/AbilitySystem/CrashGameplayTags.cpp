#include "CrashGameplayTags.h"

// Definitions of gameplay tags declared in the header file.
namespace CrashGameplayTags
{
	// Abilities
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_PersistsThroughAvatarDestruction, "Ability.Behavior.PersistsThroughAvatarDestruction", "Indicates that an ability is not canceled when its ASC’s avatar dies or is unpossessed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_InputBlocking, "Ability.Behavior.InputBlocking", "This ability blocks movement and camera input while active.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Identifier_Generic_Jump, "Ability.Identifier.Generic.Jump", "Identifier for the generic character jump ability. Includes abilities like double-jump.");


	// Effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_Damage_SelfDestruct, "Effects.Damage.SelfDestruct", "Self-destruct damage. This overrides any damage invulnerabilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_Damage_CanDamageSelf, "Effects.Damage.CanDamageSelf", "Specifies that damage caused by this gameplay effect can affect the source actor.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_Damage_CanDamageTeam, "Effects.Damage.CanDamageTeam", "Specifies that damage caused by this gameplay effect can affect the source actor's team. Does not enable damage on the source actor themselves.");


	// Events
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Death, "Event.Death", "Event triggered when an actor dies via running out of health. Activates the Death gameplay ability, defined in the game mode, on the ASC associated with the dying actor.");


	// Input
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Jump, "InputTag.Jump", "Input tag to activate the jump ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Mouse, "InputTag.Look.Mouse", "Input tag to activate the look action with a mouse.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Stick, "InputTag.Look.Stick", "Input tag to activate the look action with a gamepad.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Move, "InputTag.Move", "Input tag to activate the move action.");


	// States
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dying, "State.Dying", "The actor is currently in the process of dying. This is removed when the actor's death is finished.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_ImmuneToDamage, "State.ImmuneToDamage", "The actor is currently immune to all incoming damage. Can be overridden by effects with the SelfDestruct tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Falling, "State.Movement.Falling", "The actor is currently airborne with the \"Falling\" movement state. Does not indicate whether the actor jumped or fell. Cleared on land.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Movement_Jumping, "State.Movement.Jumping", "The actor is currently airborne as a result of jumping. Cleared on land.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_TemporarilyUnequipped, "State.TemporarilyUnequipped", "The actor has a current equipment set but it is temporarily unequipped. Actors with an EquipmentComponent will automatically unequip and re-equip their current equipment set when they gain and lose this tag, respectively.");


	// UI
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_Game, "UI.Layer.Game", "The in-game HUD. Displays health, abilities, game mode data, etc.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_GameMenu, "UI.Layer.GameMenu", "Any menus that are brought up in-game, such as the pause menu and its various sub-menus.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_Menu, "UI.Layer.Menu", "UI serving as the primary element of the game outside of gameplay, such as the main menu.");
}
