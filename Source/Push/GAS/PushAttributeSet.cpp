// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

void UPushAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, Mana, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(ThisClass, MaxMana, COND_None, REPNOTIFY_Always)
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
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
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
