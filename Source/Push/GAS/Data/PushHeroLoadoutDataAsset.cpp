// Fill out your copyright notice in the Description page of Project Settings.


#include "PushHeroLoadoutDataAsset.h"

UPA_AbilitySystemGenerics* UPushHeroLoadoutDataAsset::GetAbilitySystemGenerics() const
{
	return AbilitySystemGenerics;
}

const TMap<EAbilityInputID, FPushInputActivatedAbility>& UPushHeroLoadoutDataAsset::GetInputActivatedAbilities() const
{
	return InputActivatedAbilities;
}

const TArray<TSubclassOf<UGameplayAbility>>& UPushHeroLoadoutDataAsset::GetPassiveAbilities() const
{
	return PassiveAbilities;
}

const TArray<TSubclassOf<UGameplayEffect>>& UPushHeroLoadoutDataAsset::GetStartupEffects() const
{
	return StartupEffects;
}
