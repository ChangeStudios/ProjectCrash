// Copyright Samuel Reitich. All rights reserved.

#include "CrashGameplayTags.h"

// Definitions of gameplay tags declared in the header file.
namespace CrashGameplayTags
{
	// Abilities
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "Indicates that an ability is not canceled when its ASC’s avatar dies or is unpossessed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_InputBlocking, "Ability.Behavior.InputBlocking", "This ability blocks movement and camera input while active.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Behavior_Disabled, "Ability.Behavior.Disabled", "This ability is disabled and cannot be activated. Used to explicitly block abilities without removing them.");


	// Effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Effects_CameraType_FirstPerson, "Effects.CameraType.FirstPerson", "When this camera mode is active, first-person meshes and effects should be visible, instead of third-person.");


	// Gameplay effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_SetByCaller_Damage, "GameplayEffects.SetByCaller.Damage", "Used to dynamically set the magnitude of damage applied by gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_SetByCaller_UltimateCharge, "GameplayEffects.SetByCaller.UltimateCharge", "Used to dynamically set the ultimate charge granted by gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_SetByCaller_CooldownDuration, "GameplayEffects.SetByCaller.CooldownDuration", "Used to use a single reusable gameplay effect for ability cooldowns by dynamically setting its duration.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_CanDamageSelf, "GameplayEffects.Damage.CanDamageSelf", "The damage caused by this gameplay effect can affect the source actor.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_CanDamageTeam, "GameplayEffects.Damage.CanDamageTeam", "The damage caused by this gameplay effect can affect the source actor's team. Does not enable damage on the source actor themselves.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_FellOutOfWorld, "GameplayEffects.Damage.FellOutOfWorld", "Damage caused by falling out of the world. Overrides damage invulnerabilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_Damage_SelfDestruct, "GameplayEffects.Damage.SelfDestruct", "Self-destruct damage. Overrides damage invulnerabilities.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_UltimateCharge_FromDamage, "GameplayEffects.UltimateCharge.FromDamage", "Ultimate charge granted by dealing damage.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEffects_UltimateCharge_FromHealing, "GameplayEffects.UltimateCharge.FromHealing", "Ultimate charge granted by applying healing.");


	// Gameplay events
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_PerformTargeting, "GameplayEvent.Ability.PerformTargeting", "When this event is received during an ability that uses instant targeting, it will perform its targeting.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_StartTargeting, "GameplayEvent.Ability.StartTargeting", "Triggers an ability's targeting, usually via a target actor. Used for abilities with duration-based targeting, such as melee abilities that only hit during a certain duration in their animation. The ability's identifier should be passed in the event's TargetTags to prevent accidentally triggering other abilities that use this event.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Ability_EndTargeting, "GameplayEvent.Ability.EndTargeting", "Stops an ability's targeting, preventing it from detecting any additional hits. Used for abilities with duration-based targeting, such as melee abilities that only hit during a certain duration in their animation. The ability's identifier should be passed in the event's TargetTags to prevent accidentally affecting other abilities that use this event.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Player_Death, "GameplayEvent.Player.Death", "Triggered when an actor dies via running out of health. Can be used to trigger a \"Death\" gameplay ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayEvent_Player_Reset, "GameplayEvent.Player.Reset", "Performs an instant server reset on a target player.");


	// Game framework.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_WaitingForData, "InitState.WaitingForData", "We are entering the map and waiting to load and replicate all of the data we need for initialization: game data, Challenger data, skin data, etc.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_Initializing, "InitState.Initializing", "Actors and their components are being initialized for game play.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InitState_GameplayReady, "InitState.GameplayReady", "Ready to start gameplay at any time. All dependencies and sub-systems have been initialized.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameMode_PlayerStart_Spectator, "GameMode.PlayerStart.Spectator", "Indicates that this player start should be used for spectators.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameMode_Property_UltimateChargeRate, "GameMode.Property.UltimateChargeRate", "Scalar applied when gaining ultimate charge.");


	// Input
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Jump, "InputTag.Jump", "Input tag to activate the jump ability.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Mouse, "InputTag.Look.Mouse", "Input tag to activate the look action with a mouse.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Look_Stick, "InputTag.Look.Stick", "Input tag to activate the look action with a gamepad.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_InputTag_Move, "InputTag.Move", "Input tag to activate the move action.");


	// Messages.
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Added, "Message.Ability.Added", "A gameplay ability was granted to an ASC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Removed, "Message.Ability.Removed", "A gameplay ability was removed from an ASC.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Activated_Success, "Message.Ability.Activated.Success", "A gameplay ability was successfully activated.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Activated_Failed, "Message.Ability.Activated.Failed", "A gameplay ability tried to activate but failed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Ended, "Message.Ability.Ended", "A gameplay ability ended.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Cooldown_Started, "Message.Ability.Cooldown.Started", "A gameplay ability's cooldown started. Magnitude is the cooldown duration.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_Cooldown_Ended, "Message.Ability.Cooldown.Ended", "A gameplay ability's cooldown ended.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Ability_CostChanged, "Message.Ability.CostChanged", "The cost used to activate a gameplay ability changed. Magnitude is the cost variable's new value (e.g. new ultimate charge or new number of ability charges).");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Damage, "Message.Damage", "Verbal message communicating damage dealt.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Healing, "Message.Healing", "Verbal message communicating healing dealt.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_GameplayEffectWidget_Started, "Message.GameplayEffectWidget.Started", "Message used to initialize a new gameplay effect widget.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_GameplayEffectWidget_Updated, "Message.GameplayEffectWidget.Updated", "Message used to update an existing gameplay effect widget (e.g. when a stack is added).");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_GameplayEffectWidget_Ended, "Message.GameplayEffectWidget.Ended", "Message used to remove a gameplay effect widget when the effect has ended.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Player_Death, "Message.Player.Death", "Verbal message communicating a death.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Player_Reset, "Message.Player.Reset", "Reset message indicating a player has been directly reset.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Player_Respawn_Started, "Message.Player.Respawn.Started", "Simple duration message indicating a player began respawning.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Player_Respawn_Completed, "Message.Player.Respawn.Completed", "Simple duration message indicating a player finished respawning.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Inventory_Change, "Message.Inventory.Change", "Message communicating that an item was added to or removed from an inventory.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Message_Team_TagChange, "Message.Team.TagChange", "Message communicating that a team's count of a certain tag changed.");


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
