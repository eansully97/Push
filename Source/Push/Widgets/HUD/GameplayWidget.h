// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "GameplayWidget.generated.h"

class UStatsGauge;
class UAbilityListView;
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
	virtual void NativeDestruct() override;
	void ConfigureAbilities(const TArray<FPushInputActivatedAbilityDisplayData>& Abilities);

private:
	void BindOwnerAbilitySystemComponent();
	void StartAbilitySystemBindingRetry();
	void StopAbilitySystemBindingRetry();

	UPROPERTY(meta = (BindWidget))
	UValueGauge* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY(meta = (BindWidget))
	UAbilityListView* AbilityList;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	FTimerHandle AbilitySystemBindingRetryTimerHandle;
};
