// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PushGameplayAbility.h"
#include "GA_Infiltrate.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_Infiltrate : public UPushGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Infiltrate();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UpLaunchSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float ForwardLaunchSpeed = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Infiltrate")
	TSubclassOf<UGameplayEffect> StealthEffectClass;

	UFUNCTION()
	void StartLaunching();

	UFUNCTION()
	void OnStartStealthEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnStealthAdded();
	
	UFUNCTION()
	void OnStealthRemoved();

	bool bEndingAbility = false;
	bool bEndingFromStealthRemoval = false;

	UPROPERTY(EditDefaultsOnly, Category="Infiltrate")
	FGameplayTag StealthTag;

	UPROPERTY(EditDefaultsOnly, Category="Infiltrate")
	FGameplayTag StartStealthEventTag;
};
