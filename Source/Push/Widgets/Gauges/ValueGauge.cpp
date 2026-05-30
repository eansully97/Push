// Fill out your copyright notice in the Description page of Project Settings.


#include "ValueGauge.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(BarColor);
	}

	if (ValueText)
	{
		ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (ProgressBar)
	{
		ProgressBar->SetVisibility(bValueBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UValueGauge::NativeDestruct()
{
	ClearAttributeBindings();
	Super::NativeDestruct();
}

void UValueGauge::SetBarColor(const FLinearColor& NewColor)
{
	BarColor = NewColor;

	if (ProgressBar)
	{
		ProgressBar->SetFillColorAndOpacity(BarColor);
	}
}

void UValueGauge::SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent,
	const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	ClearAttributeBindings();

	if (AbilitySystemComponent)
	{
		bool bFoundValue = false;
		bool bFoundMaxValue = false;
		const float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFoundValue);
		const float MaxValue = AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute, bFoundMaxValue);

		if (bFoundValue && bFoundMaxValue)
		{
			SetValue(Value, MaxValue);
		}

		BoundAbilitySystemComponent = AbilitySystemComponent;
		BoundAttribute = Attribute;
		BoundMaxAttribute = MaxAttribute;
		ValueChangedDelegateHandle =
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::ValueChanged);
		MaxValueChangedDelegateHandle =
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &ThisClass::MaxValueChanged);
	}
}

void UValueGauge::SetValue(const float NewValue, const float NewMaxValue)
{
	CachedValue = NewValue;
	CachedMaxValue = NewMaxValue;
	
	if (NewMaxValue == 0)
	{
		return;
	}

	const float NewPercentage = NewValue / NewMaxValue;
	if (ProgressBar)
	{
		ProgressBar->SetPercent(NewPercentage);
	}

	const FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMaximumFractionalDigits(0);

	if (ValueText)
	{
		ValueText->SetText(FText::Format(FTextFormat::FromString("{0}/{1}"), FText::AsNumber(NewValue, &FormattingOptions), FText::AsNumber(NewMaxValue, &FormattingOptions)));
	}
}

void UValueGauge::ClearAttributeBindings()
{
	if (BoundAbilitySystemComponent)
	{
		if (ValueChangedDelegateHandle.IsValid() && BoundAttribute.IsValid())
		{
			BoundAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BoundAttribute).Remove(ValueChangedDelegateHandle);
		}

		if (MaxValueChangedDelegateHandle.IsValid() && BoundMaxAttribute.IsValid())
		{
			BoundAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BoundMaxAttribute).Remove(MaxValueChangedDelegateHandle);
		}
	}

	BoundAbilitySystemComponent = nullptr;
	BoundAttribute = FGameplayAttribute();
	BoundMaxAttribute = FGameplayAttribute();
	ValueChangedDelegateHandle.Reset();
	MaxValueChangedDelegateHandle.Reset();
}

void UValueGauge::ValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(ChangedData.NewValue, CachedMaxValue);
}

void UValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(CachedValue, ChangedData.NewValue);
}
