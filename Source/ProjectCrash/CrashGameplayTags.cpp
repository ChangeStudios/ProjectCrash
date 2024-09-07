// Copyright Samuel Reitich. All rights reserved.

#include "CrashGameplayTags.h"

// Definitions of gameplay tags declared in the header file.
namespace CrashGameplayTags
{
	// Abilities
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "Indicates that an ability is not canceled when its ASC’s avatar dies or is unpossessed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_InputBlocking, "Ability.Behavior.InputBlocking", "This ability blocks movement and camera input while active.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_Disabled, "Ability.Behavior.Disabled", "This ability is disabled and cannot be activated. Used to explicitly block abilities without removing them.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Slotted_Equipment, "Ability.Behavior.UI.Slotted.Equipment", "This ability will ability in the HUD as an equipment-granted ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Slotted_Generic, "Ability.Behavior.UI.Slotted.Generic", "This ability will ability in the HUD as a generic ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Slotted_Weapon, "Ability.Behavior.UI.Slotted.Weapon", "This ability will ability in the HUD as a weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Slotted_Ultimate, "Ability.Behavior.UI.Slotted.Ultimate", "This ability will ability in the HUD as the character's ultimate.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Frontend_PrimaryWeapon, "Ability.Behavior.UI.Frontend.PrimaryWeapon", "This ability will appear in information menus (character selection screens, character info pop-up, etc.) as a primary weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Frontend_SecondaryWeapon, "Ability.Behavior.UI.Frontend.SecondaryWeapon", "This ability will appear in information menus (character selection screens, character info pop-up, etc.) as a secondary weapon.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Frontend_StandardAbility, "Ability.Behavior.UI.Frontend.StandardAbility", "This ability will appear in information menus (character selection screens, character info pop-up, etc.) as a standard ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Frontend_Passive, "Ability.Behavior.UI.Frontend.Passive", "This ability will appear in information menus (character selection screens, character info pop-up, etc.) as a passive ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_UI_Frontend_Ultimate, "Ability.Behavior.UI.Frontend.Ultimate", "This ability will appear in information menus (character selection screens, character info pop-up, etc.) as an ultimate ability.");


	// Effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_CameraType_FirstPerson, "Effects.CameraType.FirstPerson", "When this camera mode is active, first-person meshes and effects should be visible, instead of third-person.");


	// Gameplay effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_SetByCaller_Damage, "GameplayEffects.SetByCaller.Damage", "Used to dynamically set the magnitude of damage applied by gameplay effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_CanDamageSelf, "GameplayEffects.Damage.CanDamageSelf", "The damage caused by this gameplay effect can affect the source actor.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_CanDamageTeam, "GameplayEffects.Damage.CanDamageTeam", "The damage caused by this gameplay effect can affect the source actor's team. Does not enable damage on the source actor themselves.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_FellOutOfWorld, "GameplayEffects.Damage.FellOutOfWorld", "Damage caused by falling out of the world. Overrides damage invulnerabilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_SelfDestruct, "GameplayEffects.Damage.SelfDestruct", "Self-destruct damage. Overrides damage invulnerabilities.");


	// Gameplay events
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_Death, "GameplayEvent.Ability.Death", "Event triggered when an actor dies via running out of health. Can be used to trigger a \"Death\" gameplay ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_MeleeSurfaceImpact, "GameplayEvent.Ability.MeleeSurfaceImpact", "When this event is received during a melee ability, a surface impact cue will be played if no targets have been hit yet.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_PerformTargeting, "GameplayEvent.Ability.PerformTargeting", "When this event is received during an ability that uses instant targeting, it will perform its targeting.");


	// Game framework.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_WaitingForData, "InitState.WaitingForData", "We are entering the map and waiting to load and replicate all of the data we need for initialization: game data, Challenger data, skin data, etc.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_Initializing, "InitState.Initializing", "Actors and their components are being initialized for game play.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_GameplayReady, "InitState.GameplayReady", "Ready to start gameplay at any time. All dependencies and sub-systems have been initialized.");


	// Input
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Jump, "InputTag.Jump", "Input tag to activate the jump ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Mouse, "InputTag.Look.Mouse", "Input tag to activate the look action with a mouse.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Stick, "InputTag.Look.Stick", "Input tag to activate the look action with a gamepad.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Move, "InputTag.Move", "Input tag to activate the move action.");


	// Messages.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Damage, "Message.Damage", "Verbal message communicating damage dealt.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Death, "Message.Death", "Verbal message communicating a death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Activated, "Message.Ability.Activated", "Message communicating that a gameplay ability was successfully activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Cooldown_Started, "Message.Ability.Cooldown.Started", "Message communicating that a gameplay ability's cooldown started.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Ended, "Message.Ability.Ended", "Message communicating that a gameplay ability was ended.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Failed, "Message.Ability.Failed", "Message communicating that a gameplay ability tried to activate but failed.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Inventory_Change, "Message.Inventory.Change", "Message communicating that an item was added to or removed from an inventory.");


	// States
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_AbilityInputBlocked, "State.AbilityInputBlocked", "Ability activation via input is disabled on any ASC with this tag.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dead, "State.Dead", "The player is currently dead, likely without a pawn. This is removed when the player's ASC is re-initialized with a new health component.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dying, "State.Dying", "The actor is currently in the process of dying. This is removed when the actor's death is finished.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_ImmuneToDamage, "State.ImmuneToDamage", "The actor is currently immune to all incoming damage. Can be overridden by effects with the SelfDestruct tag.");


	// UI
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_Game, "UI.Layer.Game", "The in-game HUD. Displays health, abilities, game mode data, etc.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_GameMenu, "UI.Layer.GameMenu", "Any menus that are brought up without interrupting gameplay, such as an inventory menu.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_Menu, "UI.Layer.Menu", "UI serving as the primary element of the game outside of gameplay, such as the main menu, pause menu, etc.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Layer_Modal, "UI.Layer.Modal", "An auxiliary layer for widgets that should not replace others, such as dialog prompts.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_UI_Action_Escape, "UI.Action.Escape", "Serves as a \"back\" action, and opens the escape menu during gameplay.");
}
