// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Combo.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Push/GAS/PushAbilitySystemStatics.h"


UGA_Combo::UGA_Combo()
{
	AbilityTags.AddTag(UPushAbilitySystemStatics::GetBasicAttackAbilityTag());
	BlockAbilitiesWithTag.AddTag(UPushAbilitySystemStatics::GetBasicAttackAbilityTag());
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* ComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
		ComboMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		ComboMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
		ComboMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		ComboMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		ComboMontageTask->ReadyForActivation();
	}
	
}
