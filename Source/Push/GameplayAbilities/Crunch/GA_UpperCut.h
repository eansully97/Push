// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "GA_UpperCut.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_UpperCut : public UPushGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_UpperCut();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float AbilitySweepRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float UppercutLaunchSpeed = 1000.f;
 
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float ComboSelfLaunchSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> LaunchDamageEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	TMap<FName, FGenericDamageEffectDef> ComboDamageMap;
	
	static FGameplayTag GetUpperCutLaunchTag();
	const FGenericDamageEffectDef* GetDamageEffectDefinitionForCurrentCombo() const;

	UFUNCTION()
	void StartLaunching(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboChangeEvent(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboCommitEvent(FGameplayEventData EventData);

	UFUNCTION()
	void HandleComboDamageEvent(FGameplayEventData EventData);

	FName NextComboName;
};
