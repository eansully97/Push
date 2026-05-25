// PushGameplayTags.h

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * Centralized native gameplay tags for the Push project.
 */

namespace PushGameplayTags
{
	// Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_BasicAttack);

	// Ability.Combo
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Damage);

	// Ability.Combo.Change
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Change);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Change_Combo02);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Change_Combo03);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Change_Combo04);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combo_Change_End);

	// Ability.Cooldown
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Cooldown_Crunch_Uppercut);

	// Ability.Events
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Event_Status_Launched);

	//Input
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_BasicAttack_Pressed);

	// Event.Status
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Status_Stealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Status_Launched);

	// Status
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Stun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Launched);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Stealth);

	// GameplayCue
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_CameraShake);

	// GameplayCue.Ability
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Ability_Infiltrate);

	// GameplayCue.Hit
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Crunch_Punch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Minion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Reaction);
}