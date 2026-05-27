// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_GroundBlast.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Push/PushGameplayTags.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	ActivationOwnedTags.AddTag(PushGameplayTags::Status_Aiming);
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	if (!HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
		return;

	UAbilityTask_PlayMontageAndWait* MontageWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
	MontageWaitTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->ReadyForActivation();
}
