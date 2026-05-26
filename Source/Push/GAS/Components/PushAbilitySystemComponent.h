// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushAbilitySystemComponent.generated.h"


UCLASS()
class PUSH_API UPushAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UPushAbilitySystemComponent();
	
	void ApplyInitialEffects();
	void GiveInitialAbilities();
	void ApplyFullStatEffect();
	void AuthBreakStealth();
	const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;

private:
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	bool ShouldAbilityActivationBreakStealth(const FGameplayAbilitySpecHandle Handle, const UGameplayAbility* Ability) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;
};
