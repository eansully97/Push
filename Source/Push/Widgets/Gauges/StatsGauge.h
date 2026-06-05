// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "StatsGauge.generated.h"

class UTextBlock;
class UImage;
class UAbilitySystemComponent;
class UWidgetAnimation;
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
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UImage* Icon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AttributeText;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* BlinkAnim;

	UPROPERTY(EditAnywhere, Category = "Visual")
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	FGameplayAttribute Attribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = "true"))
	bool bHideIfValueIsZero = false;

	void BindAttribute();
	void StartAttributeBindingRetry();
	void StopAttributeBindingRetry();
	void SetValue(float NewValue);
	void UpdateVisibilityForValue(float NewValue);
	void AttributeChanged(const FOnAttributeChangeData& ChangeData);
	void ClearAttributeBinding();
	FNumberFormattingOptions FormattingOptions;
	FDelegateHandle AttributeChangedDelegateHandle;
	FTimerHandle AttributeBindingRetryTimerHandle;

	UPROPERTY()
	UAbilitySystemComponent* BoundAbilitySystemComponent = nullptr;
};
