// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "ValueGauge.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class PUSH_API UValueGauge : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void SetBarColor(const FLinearColor& NewColor);
	
	void SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute);
	void SetValue(const float NewValue, const float NewMaxValue);
	
private:
	void ClearAttributeBindings();
	void ValueChanged(const FOnAttributeChangeData& ChangedData);
	void MaxValueChanged(const FOnAttributeChangeData& ChangedData);

	float CachedValue = 0.f;
	float CachedMaxValue = 0.f;
	FGameplayAttribute BoundAttribute;
	FGameplayAttribute BoundMaxAttribute;
	FDelegateHandle ValueChangedDelegateHandle;
	FDelegateHandle MaxValueChangedDelegateHandle;

	UPROPERTY()
	UAbilitySystemComponent* BoundAbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor BarColor;

	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bValueTextVisible = true;

	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bValueBarVisible = true;
	
	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	UProgressBar* ProgressBar;

	UPROPERTY(VisibleAnywhere, meta = (BindWidget))
	UTextBlock* ValueText;
};
