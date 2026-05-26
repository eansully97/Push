// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayWidget.generated.h"

class UAbilitySystemComponent;
class UValueGauge;
/**
 * 
 */
UCLASS()
class PUSH_API UGameplayWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UValueGauge* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;
};
