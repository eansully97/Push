// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_Status_Dead.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_Status_Dead : public UPushGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Status_Dead();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float RewardRange = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float BaseExperienceReward = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float BaseGoldReward = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float ExperienceRewardPerExperience = .1f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float GoldRewardPerExperience = .05f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	float KillerRewardPortion = .5f;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<UGameplayEffect> RewardEffect;
	
	TArray<AActor*> GetRewardTargets() const;
};
