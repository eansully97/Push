// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PushGameplayAbility.h"
#include "GA_UpperCut.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_UpperCut : public UPushGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float AbilitySweepRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpperCutLaunchSpeed = 1000.f;
	
	static FGameplayTag GetUpperCutLaunchTag();

	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);
};
