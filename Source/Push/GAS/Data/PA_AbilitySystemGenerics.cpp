// Fill out your copyright notice in the Description page of Project Settings.


#include "PA_AbilitySystemGenerics.h"

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetGameplayEffect(
	EPushGameplayEffectID GameplayEffectID) const
{
	const FPushGameplayEffect* GameplayEffect = GameplayEffects.FindByPredicate(
		[GameplayEffectID](const FPushGameplayEffect& Effect)
		{
			return Effect.EffectID == GameplayEffectID;
		});

	if (GameplayEffect)
	{
		return GameplayEffect->EffectClass;
	}

	return nullptr;
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetDeathEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::Death);
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetFullStatEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::FullStat);
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetHealthRegenEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::HealthRegen);
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetManaRegenEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::ManaRegen);
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetAddHeroTagEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::AddHeroTag);
}

TSubclassOf<UGameplayEffect> UPA_AbilitySystemGenerics::GetLevelStatsEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::LevelStats);
}

const TArray<TSubclassOf<UGameplayAbility>>& UPA_AbilitySystemGenerics::GetDefaultAbilities() const
{
	return DefaultAbilities;
}

const UDataTable* UPA_AbilitySystemGenerics::GetBaseDataTable() const
{
	return BaseStatsData;
}

const TArray<FPushGameplayEffect>& UPA_AbilitySystemGenerics::GetGameplayEffects() const
{
	return GameplayEffects;
}

const FRealCurve* UPA_AbilitySystemGenerics::GetExperienceCurve() const
{
	if (!ExperienceCurveTable || ExperienceRowName.IsNone())
	{
		return nullptr;
	}

	return ExperienceCurveTable->FindCurve(ExperienceRowName, "");
}
