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

int32 UPushAbilitySystemStatics::GetStaticCostForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
		return 0;

	const UGameplayEffect* CostEffect = Ability->GetCostGameplayEffect();
	if (!CostEffect || CostEffect->Modifiers.Num() == 0)
		return 0;

	float CostCost = 0;
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
