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

	OwnerAbilitySystemComponent = nullptr;
	bAbilityListConfigured = false;
	if (AbilityList)
	{
		AbilityList->ClearListItems();
	}

	InitializeFromOwner();
}

void UGameplayWidget::NativeDestruct()
{
	StopAbilitySystemBindingRetry();
	OwnerAbilitySystemComponent = nullptr;
	bAbilityListConfigured = false;
	Super::NativeDestruct();
}

void UGameplayWidget::InitializeFromOwner()
{
	UPushAbilitySystemComponent* CurrentOwnerASC = nullptr;
	if (const APushCharacter* PushCharacter = Cast<APushCharacter>(GetOwningPlayerPawn()))
	{
		CurrentOwnerASC = Cast<UPushAbilitySystemComponent>(PushCharacter->GetActivePushAbilitySystemComponent());
	}

	if (!CurrentOwnerASC)
	{
		if (OwnerAbilitySystemComponent)
		{
			if (HealthBar)
			{
				HealthBar->SetAndBoundToGameplayAttribute(nullptr, FGameplayAttribute(), FGameplayAttribute());
			}
			if (ManaBar)
			{
				ManaBar->SetAndBoundToGameplayAttribute(nullptr, FGameplayAttribute(), FGameplayAttribute());
			}
			if (AbilityList)
			{
				AbilityList->ClearListItems();
			}

			OwnerAbilitySystemComponent = nullptr;
			bAbilityListConfigured = false;
		}

		StartAbilitySystemBindingRetry();
		return;
	}

	if (OwnerAbilitySystemComponent != CurrentOwnerASC)
	{
		OwnerAbilitySystemComponent = CurrentOwnerASC;
		bAbilityListConfigured = false;

		if (AbilityList)
		{
			AbilityList->ClearListItems();
		}
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
	}

	if (!bAbilityListConfigured)
	{
		const TArray<FPushInputActivatedAbilityDisplayData> DisplayAbilities =
			OwnerAbilitySystemComponent->GetDisplayInputActivatedAbilities();
		if (AreAbilitySpecsReady(OwnerAbilitySystemComponent, DisplayAbilities))
		{
			if (AbilityList)
			{
				AbilityList->ConfigureAbilities(OwnerAbilitySystemComponent);
			}
			bAbilityListConfigured = true;
		}
	}

	if (bAbilityListConfigured)
	{
		StopAbilitySystemBindingRetry();
	}
	else
	{
		StartAbilitySystemBindingRetry();
	}
}

bool UGameplayWidget::AreAbilitySpecsReady(
	const UPushAbilitySystemComponent* AbilitySystemComponent,
	const TArray<FPushInputActivatedAbilityDisplayData>& Abilities) const
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	for (const FPushInputActivatedAbilityDisplayData& AbilityData : Abilities)
	{
		const FGameplayAbilitySpec* AbilitySpec =
			AbilitySystemComponent->FindAbilitySpecFromInputID(static_cast<int32>(AbilityData.InputID));
		if (!AbilitySpec || !AbilitySpec->Ability || AbilitySpec->Ability->GetClass() != AbilityData.AbilityClass)
		{
			return false;
		}
	}

	return true;
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
		&ThisClass::InitializeFromOwner,
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
