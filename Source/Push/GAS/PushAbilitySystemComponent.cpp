// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemComponent.h"

#include "Push/PushGameplayTags.h"
#include "Push/GameplayAbilities/GA_Infiltrate.h"
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

void UPushAbilitySystemComponent::AuthBreakStealth()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (!HasMatchingGameplayTag(PushGameplayTags::Status_Stealth))
		return;

	RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(PushGameplayTags::Status_Stealth));
}

void UPushAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (ShouldAbilityActivationBreakStealth(Handle, Ability))
	{
		AuthBreakStealth();
	}
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

bool UPushAbilitySystemComponent::ShouldAbilityActivationBreakStealth(
	const FGameplayAbilitySpecHandle Handle,
	const UGameplayAbility* Ability) const
{
	if (!Ability || Ability->IsA(UGA_Infiltrate::StaticClass()))
		return false;

	const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(Handle);
	if (!AbilitySpec)
		return false;

	const int32 BasicAttackInputID = static_cast<int32>(EAbilityInputID::BasicAttack);
	const int32 Ability6InputID = static_cast<int32>(EAbilityInputID::Ability6);

	return AbilitySpec->InputID >= BasicAttackInputID
		&& AbilitySpec->InputID <= Ability6InputID;
}
