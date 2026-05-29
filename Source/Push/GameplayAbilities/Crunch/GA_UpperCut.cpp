// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_UpperCut.h"

#include "Push/GameplayAbilities/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Push/PushGameplayTags.h"
#include "Push/Player/Characters/PushPlayerCharacter.h"

UGA_UpperCut::UGA_UpperCut()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::Ability_Crunch_Uppercut);
	SetAssetTags(AssetTags);
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
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
	return PushGameplayTags::GameplayEvent_Ability_Window_Launch;
}

const FGenericDamageEffectDef* UGA_UpperCut::GetDamageEffectDefinitionForCurrentCombo() const
{
	if (UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance())
	{
		FName CurrentComboName = OwnerAnimInstance->Montage_GetCurrentSection(AbilityMontage);
		const FGenericDamageEffectDef* EffectDef = ComboDamageMap.Find(CurrentComboName);
		return EffectDef;
	}
	return nullptr;
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

		PushTarget(AvatarActor, FVector::UpVector * UppercutLaunchSpeed);

		for (FHitResult& HitResult : TargetHitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			PushTarget(HitActor, FVector::UpVector * UppercutLaunchSpeed);
			ApplyGameplayEffectToHitResultActor(
				HitResult,
				LaunchDamageEffect,
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
	if (EventData.TargetData.Num() > 0)
	{
		return;
	}

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
		if (EventData.TargetData.Num() == 0)
		{
			return;
		}

		TArray<FHitResult> TargetHitResults =
			GetHitResultFromSweepLocationTargetData(
				EventData.TargetData,
				AbilitySweepRadius,
				ETeamAttitude::Hostile,
				ShouldDrawDebug()
			);

		AActor* AvatarActor = GetAvatarActorFromActorInfo();
		PushTarget(AvatarActor, FVector::UpVector * ComboSelfLaunchSpeed);

		const FGenericDamageEffectDef* EffectDef = GetDamageEffectDefinitionForCurrentCombo();
		if (!EffectDef)
		{
			return;
		}

		for (FHitResult& HitResult : TargetHitResults)
		{
			FVector PushVelocity = GetAvatarActorFromActorInfo()->GetActorTransform().TransformVector(EffectDef->PushVelocity);
			AActor* HitActor = HitResult.GetActor();
			PushTarget(HitActor, PushVelocity);
			ApplyGameplayEffectToHitResultActor(
				HitResult,
				EffectDef->DamageEffectClass,
				GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
			);
		}
	}
}
