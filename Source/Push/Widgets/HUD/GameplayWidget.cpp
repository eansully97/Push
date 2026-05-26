// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Push/Widgets/Gauges/ValueGauge.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	if (OwnerAbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(OwnerAbilitySystemComponent, UPushAttributeSet::GetHealthAttribute(), UPushAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(OwnerAbilitySystemComponent, UPushAttributeSet::GetManaAttribute(), UPushAttributeSet::GetMaxManaAttribute());
	}
}
