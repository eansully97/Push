// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "ValueGauge.h"
#include "Push/GAS/PushAttributeSet.h"

void UOverheadWidget::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UPushAttributeSet::GetHealthAttribute(), UPushAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UPushAttributeSet::GetManaAttribute(), UPushAttributeSet::GetMaxManaAttribute());
	}
}
