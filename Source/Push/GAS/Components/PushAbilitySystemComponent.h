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
	void RemoveTransientEffectsForDeath();
	void InitializeDefaultsFrom(const UPushAbilitySystemComponent* DefaultsSource);
	const TMap<EAbilityInputID, FPushInputActivatedAbility>& GetInputActivatedAbilities() const;
	TArray<FPushInputActivatedAbilityDisplayData> GetDisplayInputActivatedAbilities() const;
	TSubclassOf<UGameplayEffect> GetGameplayEffect(EPushGameplayEffectID GameplayEffectID) const;
	TSubclassOf<UGameplayEffect> GetDeathEffect() const;
	TSubclassOf<UGameplayEffect> GetFullStatEffect() const;
	TArray<TSubclassOf<UGameplayEffect>> GetInitialEffects() const;
	bool ValidateConfiguredData() const;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;

private:
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	bool ShouldAbilityActivationBreakStealth(const FGameplayAbilitySpecHandle Handle, const UGameplayAbility* Ability) const;
	bool ShouldPersistActiveEffectThroughDeath(const FActiveGameplayEffect& ActiveEffect) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<FPushGameplayEffect> GameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, FPushInputActivatedAbility> InputActivatedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	bool bInitialEffectsApplied = false;
	bool bInitialAbilitiesGranted = false;
};
