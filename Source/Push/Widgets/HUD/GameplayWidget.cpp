// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayWidget.h"

#include "Push/Character/Base/PushCharacter.h"
#include "Push/Widgets/Gauges/ValueGauge.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "Push/Widgets/Primitive/AbilityListView.h"
#include "TimerManager.h"

void UGameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindOwnerAbilitySystemComponent();
}

void UGameplayWidget::NativeDestruct()
{
	StopAbilitySystemBindingRetry();
	Super::NativeDestruct();
}

void UGameplayWidget::ConfigureAbilities(const TArray<FPushInputActivatedAbilityDisplayData>& Abilities)
{
	AbilityList->ConfigureAbilities(Abilities);
}

void UGameplayWidget::BindOwnerAbilitySystemComponent()
{
	UAbilitySystemComponent* CurrentOwnerASC = nullptr;
	if (const APushCharacter* PushCharacter = Cast<APushCharacter>(GetOwningPlayerPawn()))
	{
		CurrentOwnerASC = PushCharacter->GetActivePushAbilitySystemComponent();
	}

	if (!CurrentOwnerASC)
	{
		StartAbilitySystemBindingRetry();
		return;
	}

	if (OwnerAbilitySystemComponent == CurrentOwnerASC)
	{
		StopAbilitySystemBindingRetry();
		return;
	}

	OwnerAbilitySystemComponent = CurrentOwnerASC;
	if (HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(
			OwnerAbilitySystemComponent,
			UPushAttributeSet::GetHealthAttribute(),
			UPushAttributeSet::GetMaxHealthAttribute());
	}

	if (ManaBar)
	{
		ManaBar->SetAndBoundToGameplayAttribute(
			OwnerAbilitySystemComponent,
			UPushAttributeSet::GetManaAttribute(),
			UPushAttributeSet::GetMaxManaAttribute());
	}

	StopAbilitySystemBindingRetry();
}

void UGameplayWidget::StartAbilitySystemBindingRetry()
{
	UWorld* World = GetWorld();
	if (!World || AbilitySystemBindingRetryTimerHandle.IsValid())
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		AbilitySystemBindingRetryTimerHandle,
		this,
		&ThisClass::BindOwnerAbilitySystemComponent,
		0.1f,
		true);
}

void UGameplayWidget::StopAbilitySystemBindingRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AbilitySystemBindingRetryTimerHandle);
	}

	AbilitySystemBindingRetryTimerHandle.Invalidate();
}
