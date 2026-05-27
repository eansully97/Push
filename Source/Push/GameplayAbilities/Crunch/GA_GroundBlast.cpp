// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_GroundBlast.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Push/GAS/Targeting/TargetActor_GroundPick.h"
#include "Push/PushGameplayTags.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	AbilityTags.AddTag(PushGameplayTags::Ability_Crunch_GroundBlast);
	ActivationOwnedTags.AddTag(PushGameplayTags::Status_Aiming);
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
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

	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &ThisClass::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &ThisClass::TargetCancelled);
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor);
	WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	UE_LOG(LogTemp, Warning, TEXT("TargetConfirmed"));
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	UE_LOG(LogTemp, Warning, TEXT("TargetCancelled"));
	K2_EndAbility();
}
