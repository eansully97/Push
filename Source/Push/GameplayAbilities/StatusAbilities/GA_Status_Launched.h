// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_Status_Launched.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_Status_Launched : public UPushGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Status_Launched();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	static FGameplayTag GetLaunchAbilityActivationTag();
};
