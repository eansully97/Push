// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushHeroLoadoutDataAsset.generated.h"

class UPA_AbilitySystemGenerics;

UCLASS(BlueprintType)
class PUSH_API UPushHeroLoadoutDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPA_AbilitySystemGenerics* GetAbilitySystemGenerics() const;
	const TMap<EAbilityInputID, FPushInputActivatedAbility>& GetInputActivatedAbilities() const;
	const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const;
	const TArray<TSubclassOf<UGameplayEffect>>& GetStartupEffects() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	UPA_AbilitySystemGenerics* AbilitySystemGenerics = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TMap<EAbilityInputID, FPushInputActivatedAbility> InputActivatedAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;
};
