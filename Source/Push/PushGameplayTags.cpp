#include "PushGameplayTags.h"

namespace PushGameplayTags
{
	// Ability
	UE_DEFINE_GAMEPLAY_TAG(Ability, "Ability");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack, "Ability.BasicAttack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_SecondaryAttack, "Ability.SecondaryAttack");

	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack_Combo_Change, "Ability.BasicAttack.Combo.Change");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack_Combo_Change_Combo02, "Ability.BasicAttack.Combo.Change.Combo02");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack_Combo_Change_Combo03, "Ability.BasicAttack.Combo.Change.Combo03");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack_Combo_Change_Combo04, "Ability.BasicAttack.Combo.Change.Combo04");
	UE_DEFINE_GAMEPLAY_TAG(Ability_BasicAttack_Combo_Change_End, "Ability.BasicAttack.Combo.Change.End");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Crunch_Uppercut, "Ability.Crunch.Uppercut");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Crunch_GroundBlast, "Ability.Crunch.GroundBlast");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Countess_Infiltrate, "Ability.Countess.Infiltrate");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Countess_Siphon, "Ability.Countess.Siphon");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Damage_Combo, "Ability.Damage.Combo");

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
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Ability_Crunch_Uppercut_Launch, "GameplayEvent.Ability.Crunch.Uppercut.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Status_Stealth, "GameplayEvent.Status.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Status_Launched, "GameplayEvent.Status.Launched");

	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "Status.Dead");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stun, "Status.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Status_Launched, "Status.Launched");
	UE_DEFINE_GAMEPLAY_TAG(Status_Stealth, "Status.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(Status_Aiming, "Status.Aiming");

	// Gameplay Cues
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_CameraShake, "GameplayCue.CameraShake");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ability_Infiltrate, "GameplayCue.Ability.Infiltrate");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Crunch_Punch, "GameplayCue.Hit.Crunch.Punch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Crunch_GroundBlast, "GameplayCue.Hit.Crunch.GroundBlast");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Basic, "GameplayCue.Hit.Countess.Basic");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Finish, "GameplayCue.Hit.Countess.Finish");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Countess_Siphon, "GameplayCue.Hit.Countess.Siphon");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Minion, "GameplayCue.Hit.Minion");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Hit_Reaction, "GameplayCue.Hit.Reaction");
}
