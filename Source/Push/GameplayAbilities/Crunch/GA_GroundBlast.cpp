// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
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
	if (!K2_CommitAbilityCost())
	{
		K2_EndAbility();
	}
	
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

	if (ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor))
	{
		GroundPickActor->SetShouldDrawDebug(ShouldDrawDebug());
		GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
		GroundPickActor->SetTargetTraceRange(TargetTraceRange);
	}
	
	WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffectDef.DamageEffectClass, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	PushTargets(TargetDataHandle, DamageEffectDef.PushVelocity);

	FGameplayCueParameters CueParams;
	CueParams.Location = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, 1).ImpactPoint;
	CueParams.RawMagnitude = TargetAreaRadius;

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(GameplayCueTag, CueParams);
	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(PushGameplayTags::GameplayCue_CameraShake);
	
	K2_CommitAbilityCooldown();
	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}
