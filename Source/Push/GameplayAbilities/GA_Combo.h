// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PushGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_Combo : public UPushGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_Combo();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEventEndTag();
	static FGameplayTag GetComboTargetEventTag();
	
private:
	void SetupWaitComboInputPressed();
	void TryCommitCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TMap<FName, TSubclassOf<UGameplayEffect>> DamageEffectMap;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DefaultDamageEffect;

	TSubclassOf<UGameplayEffect> GetDamageEffectForCurrentCombo() const;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData Data);

	UFUNCTION()
	void DoDamage(FGameplayEventData Data);

	UFUNCTION()
	void HandleInputPress(float TimeWaited);

	FName NextComboName;
};
