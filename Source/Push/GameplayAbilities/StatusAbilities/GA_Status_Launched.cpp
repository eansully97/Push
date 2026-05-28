// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Status_Launched.h"

#include "Push/PushGameplayTags.h"

UGA_Status_Launched::UGA_Status_Launched()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	TriggerData.TriggerTag = GetLaunchAbilityActivationTag();

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::GameplayEvent_Status_Launched);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(PushGameplayTags::Status_Launched);
	ActivationBlockedTags.RemoveTag(PushGameplayTags::Status_Stun);
	AbilityTriggers.Add(TriggerData);
}

void UGA_Status_Launched::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		if (TriggerEventData && TriggerEventData->TargetData.Num() > 0)
		{
			const FHitResult* Hit = TriggerEventData->TargetData.Get(0)->GetHitResult();

			if (Hit)
			{
				const FVector LaunchVelocity =
					Hit->ImpactNormal.GetSafeNormal() * TriggerEventData->EventMagnitude;

				PushSelf(LaunchVelocity);
			}
		}

		K2_EndAbility();
	}
}

FGameplayTag UGA_Status_Launched::GetLaunchAbilityActivationTag()
{
	return PushGameplayTags::GameplayEvent_Status_Launched;
}
