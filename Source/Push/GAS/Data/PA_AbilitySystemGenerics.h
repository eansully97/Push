// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PA_AbilitySystemGenerics.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UPA_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	TSubclassOf<UGameplayEffect> GetGameplayEffect(EPushGameplayEffectID GameplayEffectID) const;
	TSubclassOf<UGameplayEffect> GetDeathEffect() const;
	TSubclassOf<UGameplayEffect> GetFullStatEffect() const;
	TSubclassOf<UGameplayEffect> GetHealthRegenEffect() const;
	TSubclassOf<UGameplayEffect> GetManaRegenEffect() const;
	TSubclassOf<UGameplayEffect> GetAddHeroTagEffect() const;
	TSubclassOf<UGameplayEffect> GetLevelStatsEffect() const;
	
	const TArray<TSubclassOf<UGameplayAbility>>& GetDefaultAbilities() const;
	const UDataTable* GetBaseDataTable() const;
	const TArray<FPushGameplayEffect>& GetGameplayEffects() const;
	const FRealCurve* GetExperienceCurve() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<FPushGameplayEffect> GameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Base Stats")
	UDataTable* BaseStatsData = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	FName ExperienceRowName = "ExperienceNeededToReachLevel";

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	UCurveTable* ExperienceCurveTable;
};
