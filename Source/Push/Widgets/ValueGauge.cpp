// Fill out your copyright notice in the Description page of Project Settings.


#include "ValueGauge.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();

	ProgressBar->SetFillColorAndOpacity(BarColor);
}

void UValueGauge::SetValue(const float NewValue, const float NewMaxValue) const
{
	if (NewMaxValue == 0)
	{
		return;
	}

	const float NewPercentage = NewValue / NewMaxValue;
	ProgressBar->SetPercent(NewPercentage);

	const FNumberFormattingOptions FormattingOptions = FNumberFormattingOptions().SetMaximumFractionalDigits(0);

	ValueText->SetText(FText::Format(FTextFormat::FromString("{0}/{1}"), FText::AsNumber(NewValue, &FormattingOptions), FText::AsNumber(NewMaxValue, &FormattingOptions)));
}
