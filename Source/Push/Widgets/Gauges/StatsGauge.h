// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "StatsGauge.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PUSH_API UStatsGauge : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AttributeText;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, Category = "Attribute")
	FGameplayAttribute Attribute;

	void SetValue(float NewValue);
	void AttributeChanged(const FOnAttributeChangeData& ChangeData);
	FNumberFormattingOptions FormattingOptions;
};
