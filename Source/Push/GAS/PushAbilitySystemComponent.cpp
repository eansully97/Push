// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemComponent.h"

#include "PushAttributeSet.h"

UPushAbilitySystemComponent::UPushAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HealthUpdated);
}

void UPushAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	for (const auto& EffectClass : InitialEffects)
	{
		AuthApplyGameplayEffect(EffectClass);
	}
}

void UPushAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	for (const auto& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0, static_cast<int32>(AbilityPair.Key), nullptr));
	}

	for (const auto& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1, static_cast<int32>(AbilityPair.Key), nullptr));
	}
}

void UPushAbilitySystemComponent::ApplyFullStatEffect()
{
	AuthApplyGameplayEffect(FullStatEffect);
}

void UPushAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner()) return;

	if (ChangeData.NewValue <= 0 && GetOwner()->HasAuthority() && DeathEffect)
	{
		AuthApplyGameplayEffect(DeathEffect);
	}
}

void UPushAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
	ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}
