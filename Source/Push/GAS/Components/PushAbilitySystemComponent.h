// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushAbilitySystemComponent.generated.h"


class UPA_AbilitySystemGenerics;

UCLASS()
class PUSH_API UPushAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UPushAbilitySystemComponent();

	bool InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	void AuthApplyDeathStatusEffect();
	void AuthBreakStealth();
	void RemoveTransientEffectsForDeath();
	void InitializeDefaultsFrom(const UPushAbilitySystemComponent* DefaultsSource);
	const TMap<EAbilityInputID, FPushInputActivatedAbility>& GetInputActivatedAbilities() const;
	TArray<FPushInputActivatedAbilityDisplayData> GetDisplayInputActivatedAbilities() const;
	bool IsAtMaxLevel() const;
	bool ValidateConfiguredData() const;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;

private:
	void ApplyPostStartupEffects(bool bStartupAttributesInitialized);
	void GiveInitialAbilities();
	void BindHealthAttributeDelegate();
	void BindManaAttributeDelegate();
	void BindExperienceAttributeDelegate();
	bool ValidateConfiguredDataOnce();
	bool ValidateStartupConfiguration() const;
	bool ValidateBaseStatsConfiguration(bool bBaseAttributesExpected, const AActor* StatsActor) const;
	const FHeroBaseStats* FindBaseStatsForActor(const AActor* StatsActor) const;
	FString GetValidationContext() const;
	bool HasStartupEffects() const;
	bool ApplyConfiguredStartupEffects();
	bool HasHeroAttributes() const;
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	void ExperienceUpdated(const FOnAttributeChangeData& ChangeData);
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	bool ShouldAbilityActivationBreakStealth(const FGameplayAbilitySpecHandle Handle, const UGameplayAbility* Ability) const;
	bool ShouldPersistActiveEffectThroughDeath(const FActiveGameplayEffect& ActiveEffect) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, FPushInputActivatedAbility> InputActivatedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	UPA_AbilitySystemGenerics* AbilitySystemGenerics;

	bool bStartupEffectsApplied = false;
	bool bInitialAbilitiesGranted = false;
	bool bConfiguredDataValidated = false;
	bool bConfiguredDataValid = false;
	bool bHealthAttributeDelegateBound = false;
	bool bManaAttributeDelegateBound = false;
	bool bExperienceAttributeDelegateBound = false;
	FDelegateHandle HealthAttributeChangedDelegateHandle;
	FDelegateHandle ManaAttributeChangedDelegateHandle;
	FDelegateHandle ExperienceAttributeChangedDelegateHandle;
};
