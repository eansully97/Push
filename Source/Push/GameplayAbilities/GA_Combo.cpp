// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Combo.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Push/PushGameplayTags.h"


UGA_Combo::UGA_Combo()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::Ability_BasicAttack);
	SetAssetTags(AssetTags);
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability_BasicAttack);
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!AbilityMontage || !GetOwnerAnimInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot activate combo: AbilityMontage=%s AnimInstance=%s."),
			*GetName(),
			*GetNameSafe(AbilityMontage),
			*GetNameSafe(GetOwnerAnimInstance()));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

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

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &ThisClass::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	if (K2_HasAuthority())
	{
		UAbilityTask_WaitGameplayEvent* WaitTargetingEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboTargetEventTag());
		WaitTargetingEventTask->EventReceived.AddDynamic(this, &ThisClass::DoDamage);
		WaitTargetingEventTask->ReadyForActivation();
	}
	
	SetupWaitComboInputPressed();
}

void UGA_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	NextComboName = NAME_None;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return PushGameplayTags::GameplayEvent_Ability_Combo_Change;
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return PushGameplayTags::GameplayEvent_Ability_Combo_Change_End;
}

FGameplayTag UGA_Combo::GetComboTargetEventTag()
{
	return PushGameplayTags::GameplayEvent_Ability_Combo_Damage;
}

void UGA_Combo::SetupWaitComboInputPressed()
{
	UAbilityTask_WaitInputPress* WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPress->OnPress.AddDynamic(this, &ThisClass::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None || !AbilityMontage)
		return;

	UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance();
	if (!OwnerAnimInstance)
		return;

	OwnerAnimInstance->Montage_SetNextSection(OwnerAnimInstance->Montage_GetCurrentSection(AbilityMontage), NextComboName, AbilityMontage);
}

TSubclassOf<UGameplayEffect> UGA_Combo::GetDamageEffectForCurrentCombo() const
{
	if (UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance())
	{
		FName CurrentSectionName = OwnerAnimInstance->Montage_GetCurrentSection(AbilityMontage);
		if (const TSubclassOf<UGameplayEffect>* FoundEffectPtr = DamageEffectMap.Find(CurrentSectionName))
		{
			return *FoundEffectPtr;
		}
	}
	return DefaultDamageEffect;
}


void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	if (Data.TargetData.Num() > 0)
	{
		return;
	}

	FGameplayTag EventTag = Data.EventTag;

	if (EventTag == GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}

	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
	NextComboName = TagNames.Last();
}

void UGA_Combo::DoDamage(FGameplayEventData Data)
{
	if (Data.TargetData.Num() == 0)
	{
		return;
	}

	TArray<FHitResult> HitResults = GetHitResultFromSweepLocationTargetData(Data.TargetData, TargetSweepSphereRadius, ETeamAttitude::Hostile, ShouldDrawDebug());

	for (const auto& HitResult : HitResults)
	{
		TSubclassOf<UGameplayEffect> GameplayEffect = GetDamageEffectForCurrentCombo();
		ApplyGameplayEffectToHitResultActor(HitResult, GameplayEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
	}
}

void UGA_Combo::HandleInputPress(float TimeWaited)
{
	SetupWaitComboInputPressed();
	TryCommitCombo();
}
