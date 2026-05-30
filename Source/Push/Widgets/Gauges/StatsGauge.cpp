// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

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
	
	if (APawn* OwnerPlayerPawn = GetOwningPlayerPawn())
	{
		if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn))
		{
			bool bFound = false;
			const float AttributeValue = OwnerASC->GetGameplayAttributeValue(Attribute, bFound);
			if (bFound)
			{
				SetValue(AttributeValue);
			}

			ClearAttributeBinding();
			BoundAbilitySystemComponent = OwnerASC;
			AttributeChangedDelegateHandle =
				OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::AttributeChanged);
		}
	}
}

void UStatsGauge::NativeDestruct()
{
	ClearAttributeBinding();
	Super::NativeDestruct();
}

void UStatsGauge::SetValue(float NewValue)
{
	if (AttributeText)
	{
		AttributeText->SetText(FText::AsNumber(NewValue, &FormattingOptions));
	}
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
