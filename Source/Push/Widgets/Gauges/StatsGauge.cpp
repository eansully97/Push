// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Push/Character/Base/PushCharacter.h"
#include "TimerManager.h"

void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (Icon)
	{
		Icon->SetBrushFromTexture(IconTexture);
	}
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();

	FormattingOptions.MaximumFractionalDigits = 0;

	BindAttribute();
}

void UStatsGauge::NativeDestruct()
{
	StopAttributeBindingRetry();
	ClearAttributeBinding();
	Super::NativeDestruct();
}

void UStatsGauge::BindAttribute()
{
	if (!Attribute.IsValid())
	{
		StopAttributeBindingRetry();
		return;
	}

	UAbilitySystemComponent* OwnerASC = nullptr;
	if (const APushCharacter* PushCharacter = Cast<APushCharacter>(GetOwningPlayerPawn()))
	{
		OwnerASC = PushCharacter->GetActivePushAbilitySystemComponent();
	}

	if (!OwnerASC)
	{
		StartAttributeBindingRetry();
		return;
	}

	if (BoundAbilitySystemComponent == OwnerASC && AttributeChangedDelegateHandle.IsValid())
	{
		StopAttributeBindingRetry();
		return;
	}

	ClearAttributeBinding();

	bool bFound = false;
	const float AttributeValue = OwnerASC->GetGameplayAttributeValue(Attribute, bFound);
	if (bFound)
	{
		SetValue(AttributeValue);
	}

	BoundAbilitySystemComponent = OwnerASC;
	AttributeChangedDelegateHandle =
		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::AttributeChanged);

	StopAttributeBindingRetry();
}

void UStatsGauge::StartAttributeBindingRetry()
{
	UWorld* World = GetWorld();
	if (!World || AttributeBindingRetryTimerHandle.IsValid())
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		AttributeBindingRetryTimerHandle,
		this,
		&ThisClass::BindAttribute,
		0.1f,
		true);
}

void UStatsGauge::StopAttributeBindingRetry()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttributeBindingRetryTimerHandle);
	}

	AttributeBindingRetryTimerHandle.Invalidate();
}

void UStatsGauge::SetValue(float NewValue)
{
	if (AttributeText)
	{
		AttributeText->SetText(FText::AsNumber(NewValue, &FormattingOptions));
	}

	UpdateVisibilityForValue(NewValue);
}

void UStatsGauge::UpdateVisibilityForValue(float NewValue)
{
	SetVisibility(bHideIfValueIsZero && FMath::IsNearlyZero(NewValue)
		? ESlateVisibility::Collapsed
		: ESlateVisibility::Visible);
}

void UStatsGauge::AttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	SetValue(ChangeData.NewValue);
}

void UStatsGauge::ClearAttributeBinding()
{
	if (BoundAbilitySystemComponent && AttributeChangedDelegateHandle.IsValid() && Attribute.IsValid())
	{
		BoundAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(AttributeChangedDelegateHandle);
	}

	BoundAbilitySystemComponent = nullptr;
	AttributeChangedDelegateHandle.Reset();
}
