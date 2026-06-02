// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"

void UPushAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Mana, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxMana, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, AttackDamage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, SpellPower, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Armor, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, SpellResist, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MoveSpeed, COND_None, REPNOTIFY_Always)
}

void UPushAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
}

void UPushAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const bool bTookDamage = Data.EvaluatedData.Magnitude < 0.f;
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
		const float CurrentMaxHealth = GetMaxHealth();
		SetCachedHealthPercent(CurrentMaxHealth > 0.f ? GetHealth() / CurrentMaxHealth : 0.f);
		if (bTookDamage)
		{
			if (UPushAbilitySystemComponent* PushASC = Cast<UPushAbilitySystemComponent>(&Data.Target))
			{
				PushASC->AuthBreakStealth();
			}
		}
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
		const float CurrentMaxMana = GetMaxMana();
		SetCachedManaPercent(CurrentMaxMana > 0.f ? GetMana() / CurrentMaxMana : 0.f);
	}
}

void UPushAttributeSet::RescaleHealth()
{
	if (!GetOwningActor()->HasAuthority())
		return;

	if (GetCachedHealthPercent() != 0 && GetHealth() != 0)
	{
		SetHealth(GetMaxHealth() * GetCachedHealthPercent());
	}
}

void UPushAttributeSet::RescaleMana()
{
	if (!GetOwningActor()->HasAuthority())
		return;

	if (GetCachedManaPercent() != 0 && GetMana() != 0)
	{
		SetMana(GetMaxMana() * GetCachedManaPercent());
	}
}

void UPushAttributeSet::OnRep_Health(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, Health, LastValue)
}

void UPushAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, MaxHealth, LastValue)
}

void UPushAttributeSet::OnRep_Mana(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, Mana, LastValue)
}

void UPushAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, MaxMana, LastValue)
}

void UPushAttributeSet::OnRep_AttackDamage(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, AttackDamage, LastValue)
}

void UPushAttributeSet::OnRep_SpellPower(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, SpellPower, LastValue)
}

void UPushAttributeSet::OnRep_Armor(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, Armor, LastValue)
}

void UPushAttributeSet::OnRep_SpellResist(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, SpellResist, LastValue)
}

void UPushAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& LastValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPushAttributeSet, MoveSpeed, LastValue)
}
