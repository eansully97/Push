// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Status_Dead.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/OverlapResult.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/PushAbilitySystemStatics.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"

UGA_Status_Dead::UGA_Status_Dead()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = PushGameplayTags::Status_Dead;
	
	AbilityTriggers.Add(TriggerData);
	ActivationBlockedTags.RemoveTag(PushGameplayTags::Status_Stun);
}

void UGA_Status_Dead::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_HasAuthority())
	{
		AActor* Killer = TriggerEventData->ContextHandle.GetEffectCauser();
		if ( !Killer || !UPushAbilitySystemStatics::IsHero(Killer))
		{
			Killer = nullptr;
		}

		TArray<AActor*> RewardTargets = GetRewardTargets();
		if (RewardTargets.Num() == 0 && !Killer)
		{
			K2_EndAbility();
			return;
		}

		if (Killer && !RewardTargets.Contains(Killer))
		{
			RewardTargets.Add(Killer);
		}
		bool bFound = false;
		float SelfExperience = GetAbilitySystemComponentFromActorInfo_Ensured()->GetGameplayAttributeValue(UPushHeroAttributeSet::GetExperienceAttribute(), bFound);
		
		float TotalExperienceReward = BaseExperienceReward + ExperienceRewardPerExperience * SelfExperience;
		float TotalGoldReward = BaseGoldReward + GoldRewardPerExperience * SelfExperience;

		if (Killer)
		{
			float KillerExperienceReward = TotalExperienceReward * KillerRewardPortion;
			float KillerGoldReward = TotalGoldReward * KillerRewardPortion;

			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(RewardEffect);
			EffectSpecHandle.Data->SetSetByCallerMagnitude(PushGameplayTags::Data_Value_Experience, KillerExperienceReward);
			EffectSpecHandle.Data->SetSetByCallerMagnitude(PushGameplayTags::Data_Value_Gold, KillerGoldReward);
			K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Killer));

			TotalExperienceReward -= KillerExperienceReward;
			TotalGoldReward -= KillerGoldReward;
		}

		float ExperiencePerTarget = TotalExperienceReward / RewardTargets.Num();
		float GoldPerTarget = TotalGoldReward / RewardTargets.Num();

		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(RewardEffect);
		EffectSpecHandle.Data->SetSetByCallerMagnitude(PushGameplayTags::Data_Value_Experience, ExperiencePerTarget);
		EffectSpecHandle.Data->SetSetByCallerMagnitude(PushGameplayTags::Data_Value_Gold, GoldPerTarget);
		K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(RewardTargets, true));
	}
	K2_EndAbility();
}

TArray<AActor*> UGA_Status_Dead::GetRewardTargets() const
{
	TSet<AActor*> RewardTargets;
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();

	if (!AvatarActor || !GetWorld())
	{
		return RewardTargets.Array();
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	TArray<FOverlapResult> OverlapResults;
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(RewardRange);
	if (GetWorld()->OverlapMultiByObjectType(OverlapResults, AvatarActor->GetActorLocation(), FQuat::Identity, ObjectQueryParams, Sphere))
	{
		for (const auto& OverlapResult : OverlapResults)
		{
			const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(OverlapResult.GetActor());
			if (!OtherTeamInterface || OtherTeamInterface->GetTeamAttitudeTowards(*AvatarActor) != ETeamAttitude::Hostile)
			{
				continue;
			}

			if (!UPushAbilitySystemStatics::IsHero(OverlapResult.GetActor()))
			{
				continue;
			}

			RewardTargets.Add(OverlapResult.GetActor());
		}
	}
	return RewardTargets.Array();
}
