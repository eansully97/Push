// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemStatics.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Abilities/GameplayAbility.h"
#include "Push/PushGameplayTags.h"

float UPushAbilitySystemStatics::GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
	return 0;

	const UGameplayEffect* CooldownEffect = Ability->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0;

	float CooldownDuration = 0.f;
	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1, CooldownDuration);
	return CooldownDuration;
}

float UPushAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0.f;

	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
		return 0.f;

	float CostCost = 0.f;
	CostEffect->Modifiers[0].ModifierMagnitude.GetStaticMagnitudeIfPossible(1, CostCost);
	return FMath::Abs(CostCost);
}

bool UPushAbilitySystemStatics::IsHero(const AActor* ActorToCheck)
{
	if (const IAbilitySystemInterface* ActorASI = Cast<IAbilitySystemInterface>(ActorToCheck))
	{
		if (UAbilitySystemComponent* ActorASC = ActorASI->GetAbilitySystemComponent())
		{
			return ActorASC->HasMatchingGameplayTag(PushGameplayTags::Status_Hero);
		}
	}
	return false;
}

bool UPushAbilitySystemStatics::IsAbilityMaxLevel(const FGameplayAbilitySpec& AbilitySpec)
{
	return AbilitySpec.Level >= 4;
}

bool UPushAbilitySystemStatics::CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec,
	const UAbilitySystemComponent& ASC)
{
	if (const UGameplayAbility* AbilityCDO = AbilitySpec.Ability)
	{
		return AbilityCDO->CheckCost(AbilitySpec.Handle, ASC.AbilityActorInfo.Get());
	}
	return false;
}

float UPushAbilitySystemStatics::GetManaCostFor(const UGameplayAbility* AbilityObj,
	const UAbilitySystemComponent& ASC, int32 AbilityLevel)
{
	float ManaCost = 0.f;
	if (AbilityObj)
	{
		UGameplayEffect* CostEffect = AbilityObj->GetCostGameplayEffect();
		if (CostEffect)
		{
			FGameplayEffectSpecHandle EffectSpec = ASC.MakeOutgoingSpec(CostEffect->GetClass(), AbilityLevel, ASC.MakeEffectContext());
			CostEffect->Modifiers[0].ModifierMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(), ManaCost);
		}
	}
	return FMath::Abs(ManaCost);
}

float UPushAbilitySystemStatics::GetCooldownDurationFor(const UGameplayAbility* AbilityObj,
	const UAbilitySystemComponent& ASC, int32 AbilityLevel)
{
	float Cooldown = 0.f;
	if (AbilityObj)
	{
		UGameplayEffect* CooldownEffect = AbilityObj->GetCooldownGameplayEffect();
		if (CooldownEffect)
		{
			FGameplayEffectSpecHandle EffectSpec = ASC.MakeOutgoingSpec(CooldownEffect->GetClass(), AbilityLevel, ASC.MakeEffectContext());
			CooldownEffect->DurationMagnitude.AttemptCalculateMagnitude(*EffectSpec.Data.Get(), Cooldown);
		}
	}
	return FMath::Abs(Cooldown);
}

float UPushAbilitySystemStatics::GetCooldownRemainingFor(const UGameplayAbility* AbilityObj,
	const UAbilitySystemComponent& ASC)
{
	if (!AbilityObj)
		return 0.f;

	UGameplayEffect* CooldownEffect = AbilityObj->GetCooldownGameplayEffect();
	if (!CooldownEffect)
		return 0.f;

	FGameplayEffectQuery CooldownEffectQuery;
	CooldownEffectQuery.EffectDefinition = CooldownEffect->GetClass();

	float CooldownRemaining = 0.f;
	FJsonSerializableArrayFloat CooldownTimesRemaining = ASC.GetActiveEffectsTimeRemaining(CooldownEffectQuery);

	for (float Remaining : CooldownTimesRemaining)
	{
		if (Remaining > CooldownRemaining)
		{
			CooldownRemaining = Remaining;
		}
	}
	return CooldownRemaining;
}
