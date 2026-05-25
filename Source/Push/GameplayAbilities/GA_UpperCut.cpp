// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_UpperCut.h"

#include "GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Push/PushGameplayTags.h"
#include "Push/Player/PushPlayerCharacter.h"

UGA_UpperCut::UGA_UpperCut()
{
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability_BasicAttack);
}

void UGA_UpperCut::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* AbilityMontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AbilityMontage);

		AbilityMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
		AbilityMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitLaunchEventTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetUpperCutLaunchTag());

		WaitLaunchEventTask->EventReceived.AddDynamic(this, &ThisClass::StartLaunching);
		WaitLaunchEventTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEvent =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this,
				UGA_Combo::GetComboChangedEventTag(),
				nullptr,
				false,
				false
			);

		WaitComboChangeEvent->EventReceived.AddDynamic(this, &ThisClass::HandleComboChangeEvent);
		WaitComboChangeEvent->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboCommitEvent =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, PushGameplayTags::Input_Ability_BasicAttack_Pressed);

		WaitComboCommitEvent->EventReceived.AddDynamic(this, &ThisClass::HandleComboCommitEvent);
		WaitComboCommitEvent->ReadyForActivation();
	}

	NextComboName = NAME_None;
}

FGameplayTag UGA_UpperCut::GetUpperCutLaunchTag()
{
	return PushGameplayTags::Ability_Event_Status_Launched;
}

void UGA_UpperCut::StartLaunching(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults =
			GetHitResultFromSweepLocationTargetData(
				EventData.TargetData,
				AbilitySweepRadius,
				ETeamAttitude::Hostile,
				ShouldDrawDebug()
			);

		AActor* AvatarActor = GetAvatarActorFromActorInfo();

		PushTarget(AvatarActor, FVector::UpVector * UpLaunchSpeed);

		for (FHitResult& HitResult : TargetHitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			PushTarget(HitActor, FVector::UpVector * UpLaunchSpeed);
			ApplyGameplayEffectToHitResultActor(
				HitResult,
				DamageEffect,
				GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
			);
		}
		UAbilityTask_WaitGameplayEvent* WaitComboDamageEvent =
				UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
					this,
					UGA_Combo::GetComboTargetEventTag()
				);

		WaitComboDamageEvent->EventReceived.AddDynamic(
			this,
			&ThisClass::HandleComboDamageEvent
		);

		WaitComboDamageEvent->ReadyForActivation();
	}
}

void UGA_UpperCut::HandleComboChangeEvent(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;

	if (EventTag == UGA_Combo::GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

	if (TagNames.Num() > 0)
	{
		NextComboName = TagNames.Last();
	}
}

void UGA_UpperCut::HandleComboCommitEvent(FGameplayEventData EventData)
{
	if (NextComboName == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpperCut: Cannot commit combo. NextComboName is NAME_None."));
		return;
	}

	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();

	if (!OwnerAnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpperCut: Cannot commit combo. OwnerAnimInstance is null."));
		return;
	}

	FName CurrentSection = OwnerAnimInstance->Montage_GetCurrentSection(AbilityMontage);
	OwnerAnimInstance->Montage_SetNextSection(CurrentSection, NextComboName, AbilityMontage);
}

void UGA_UpperCut::HandleComboDamageEvent(FGameplayEventData EventData)
{
	if (K2_HasAuthority())
	{
		TArray<FHitResult> TargetHitResults =
			GetHitResultFromSweepLocationTargetData(
				EventData.TargetData,
				AbilitySweepRadius,
				ETeamAttitude::Hostile,
				ShouldDrawDebug()
			);

		for (FHitResult& HitResult : TargetHitResults)
		{
			ApplyGameplayEffectToHitResultActor(
				HitResult,
				DamageEffect,
				GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
			);
		}
	}
}
