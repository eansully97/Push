#include "PushGameplayTags.h"

namespace PushGameplayTags
{
	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack, "Ability.BasicAttack");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Crunch_Uppercut, "Ability.Crunch.Uppercut");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Crunch_GroundBlast, "Ability.Crunch.GroundBlast");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Countess_Infiltrate, "Ability.Countess.Infiltrate");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Countess_Siphon, "Ability.Countess.Siphon");

	// Cooldown
	UE_DEFINE_GAMEPLAY_TAG(Cooldown, "Cooldown");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Crunch_Uppercut, "Cooldown.Ability.Crunch.Uppercut");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Crunch_GroundBlast, "Cooldown.Ability.Crunch.GroundBlast");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Countess_Infiltrate, "Cooldown.Ability.Countess.Infiltrate");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Ability_Countess_Siphon, "Cooldown.Ability.Countess.Siphon");

	// Input
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_BasicAttack_Pressed, "Input.Ability.BasicAttack.Pressed");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_SecondaryAttack_Pressed, "Input.Ability.SecondaryAttack.Pressed");

	// Gameplay Events
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Window_Launch, "GameplayEvent.Ability.Window.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Window_Stealth, "GameplayEvent.Ability.Window.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Change, "GameplayEvent.Ability.Combo.Change");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Change_Combo02, "GameplayEvent.Ability.Combo.Change.Combo02");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Change_Combo03, "GameplayEvent.Ability.Combo.Change.Combo03");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Change_Combo04, "GameplayEvent.Ability.Combo.Change.Combo04");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Change_End, "GameplayEvent.Ability.Combo.Change.End");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Combo_Damage, "GameplayEvent.Ability.Combo.Damage");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Status_Launch, "GameplayEvent.Status.Launch");

	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "Status.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Status_Launched, "Status.Launched");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stealth, "Status.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(Status_Aiming, "Status.Aiming");

	// Gameplay Cues
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_CameraShake, "GameplayCue.CameraShake");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Status_Stealth, "GameplayCue.Status.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Status_Stealth_Countess_Infiltrate, "GameplayCue.Status.Stealth.Countess.Infiltrate");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Crunch_Punch, "GameplayCue.Hit.Crunch.Punch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Crunch_GroundBlast, "GameplayCue.Hit.Crunch.GroundBlast");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Basic, "GameplayCue.Hit.Countess.Basic");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Finish, "GameplayCue.Hit.Countess.Finish");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Siphon, "GameplayCue.Hit.Countess.Siphon");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Minion, "GameplayCue.Hit.Minion");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Reaction, "GameplayCue.Hit.Reaction");
}
