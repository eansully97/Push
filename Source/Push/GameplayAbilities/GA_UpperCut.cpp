// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_UpperCut.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UGA_UpperCut::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* AbilityMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);
		AbilityMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());
		WaitLaunchEventTask->EventReceived.AddDynamic(this, &ThisClass::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();
	}
}

FGameplayTag UGA_UpperCut::GetUpperCutLaunchTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.UpperCut.Launch");
}

void UGA_UpperCut::StartLaunching(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults = GetHitResultFromSweepLocationTargetData(EventData.TargetData, AbilitySweepRadius, ETeamAttitude::Hostile, ShouldDrawDebug());
		PushTarget(GetAvatarActorFromActorInfo(), FVector::UpVector * UpperCutLaunchSpeed);
		for (FHitResult& HitResult : TargetHitResults)
		{
			PushTarget(HitResult.GetActor(), FVector::UpVector * UpperCutLaunchSpeed);
		}
	}
}
