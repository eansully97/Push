// PushGameplayTags.cpp

#include "PushGameplayTags.h"

namespace PushGameplayTags
{
	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack, "Ability.BasicAttack");

	// Ability.Combo
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Damage, "Ability.Combo.Damage");

	// Ability.Combo.Change
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Change, "Ability.Combo.Change");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Change_Combo02, "Ability.Combo.Change.Combo02");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Change_Combo03, "Ability.Combo.Change.Combo03");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Change_Combo04, "Ability.Combo.Change.Combo04");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Combo_Change_End, "Ability.Combo.Change.End");

	// Ability.Cooldown
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cooldown_Crunch_Uppercut, "Ability.Cooldown.Crunch.Uppercut");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cooldown_Countess_Infiltrate, "Ability.Cooldown.Countess.Infiltrate");

	// Ability.Events
	UE_DEFINE_GAMEPLAY_TAG(Ability_Event_Status_Launched, "Ability.Event.Status.Launched");

	//Input
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_BasicAttack_Pressed, "Input.Ability.BasicAttack.Pressed");
	
	// Event.Status
	UE_DEFINE_GAMEPLAY_TAG(Event_Status_Launched, "Event.Status.Launched");
	UE_DEFINE_GAMEPLAY_TAG(Event_Status_Stealth, "Event.Status.Stealth");
	
	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "Status.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Status_Launched, "Status.Launched");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stealth, "Status.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(Status_Aiming, "Status.Aiming");

	// GameplayCue
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_CameraShake, "GameplayCue.CameraShake");

	// GameplayCue.Ability
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ability_Infiltrate, "GameplayCue.Ability.Infiltrate");

	// GameplayCue.Hit
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Crunch_Punch, "GameplayCue.Hit.Crunch.Punch");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Basic, "GameplayCue.Hit.Countess.Basic");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Finish, "GameplayCue.Hit.Countess.Finish");
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Minion, "GameplayCue.Hit.Minion");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Reaction, "GameplayCue.Hit.Reaction");
}