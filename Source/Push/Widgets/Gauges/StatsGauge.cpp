// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	Icon->SetBrushFromTexture(IconTexture);
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();

	FormattingOptions.MaximumFractionalDigits = 0;
	
	if (APawn* OwnerPlayerPawn = GetOwningPlayerPawn())
	{
		if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn))
		{
			bool bFound;
			float AttributeValue = OwnerASC->GetGameplayAttributeValue(Attribute, bFound);
			SetValue(AttributeValue);

			OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::AttributeChanged);
		}
	}
}

void UStatsGauge::SetValue(float NewValue)
{
	AttributeText->SetText(FText::AsNumber(NewValue, &FormattingOptions));
}

void UStatsGauge::AttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	SetValue(ChangeData.NewValue);
}
