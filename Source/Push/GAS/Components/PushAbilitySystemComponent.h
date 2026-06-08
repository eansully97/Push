// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushAbilitySystemComponent.generated.h"


class UPA_AbilitySystemGenerics;
class UPushHeroLoadoutDataAsset;

UCLASS()
class PUSH_API UPushAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	UPushAbilitySystemComponent();

	bool InitializeBaseAttributes();
	void ServerSideInit();
	void ApplyFullStatEffect();
	void ApplyRespawnStatEffects();
	void AuthApplyDeathStatusEffect();
	void AuthBreakStealth();
	void RemoveTransientEffectsForDeath();
	void InitializeDefaultsFrom(const UPushAbilitySystemComponent* DefaultsSource);
	void InitializeDefaultsFromLoadout(const UPushHeroLoadoutDataAsset* Loadout);
	const TMap<EAbilityInputID, FPushInputActivatedAbility>& GetInputActivatedAbilities() const;
	TArray<FPushInputActivatedAbilityDisplayData> GetDisplayInputActivatedAbilities() const;
	bool IsAtMaxLevel() const;
	bool ValidateConfiguredData() const;
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpgradeAbilityWithID(EAbilityInputID InputID);

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void OnRep_ActivateAbilities() override;

private:
	void ApplyPostStartupEffects(bool bStartupAttributesInitialized);
	void GiveInitialAbilities();
	void BindHealthAttributeDelegate();
	void BindManaAttributeDelegate();
	void BindExperienceAttributeDelegate();
	void InitializeDefaultConfiguration(
		const TMap<EAbilityInputID, FPushInputActivatedAbility>& NewInputActivatedAbilities,
		const TArray<TSubclassOf<UGameplayAbility>>& NewPassiveAbilities,
		const TArray<TSubclassOf<UGameplayEffect>>& NewStartupEffects,
		UPA_AbilitySystemGenerics* NewAbilitySystemGenerics);
	bool ValidateConfiguredDataOnce();
	bool ValidateStartupConfiguration() const;
	bool ValidateBaseStatsConfiguration(bool bBaseAttributesExpected, const AActor* StatsActor) const;
	const FHeroBaseStats* FindBaseStatsForActor(const AActor* StatsActor) const;
	FString GetValidationContext() const;
	bool HasStartupEffects() const;
	bool ApplyConfiguredStartupEffects();
	bool HasHeroAttributes() const;
	void RefreshLevelForExperience(float CurrentExperience);
	void HealthUpdated(const FOnAttributeChangeData& ChangeData);
	void ManaUpdated(const FOnAttributeChangeData& ChangeData);
	void ExperienceUpdated(const FOnAttributeChangeData& ChangeData);
	void AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	void AuthApplyGameplayEffectIfMissing(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level = 1);
	bool HasActiveGameplayEffectOfClass(TSubclassOf<UGameplayEffect> GameplayEffect) const;
	bool IsPersistentBaselineEffect(TSubclassOf<UGameplayEffect> GameplayEffectClass, const FActiveGameplayEffect& ActiveEffect) const;
	bool ShouldAbilityActivationBreakStealth(const FGameplayAbilitySpecHandle Handle, const UGameplayAbility* Ability) const;
	bool ShouldPersistActiveEffectThroughDeath(const FActiveGameplayEffect& ActiveEffect) const;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (ToolTip = "Input-bound abilities granted by this ASC. Player characters currently copy these defaults from the pawn ASC to the PlayerState ASC."))
	TMap<EAbilityInputID, FPushInputActivatedAbility> InputActivatedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (ToolTip = "Non-input default abilities granted during server ASC startup. Prefer this over input id None."))
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta = (ToolTip = "Optional designer-provided startup effects. If omitted, base stats are initialized from AbilitySystemGenerics data."))
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta = (ToolTip = "Shared effects, default abilities, base stats, and level curve data for this ASC setup."))
	UPA_AbilitySystemGenerics* AbilitySystemGenerics;

	bool bStartupEffectsApplied = false;
	bool bBaseAttributesInitialized = false;
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
