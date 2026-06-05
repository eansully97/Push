// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "GameplayWidget.generated.h"

class UStatsGauge;
class UAbilityListView;
class UPushAbilitySystemComponent;
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

private:
	void InitializeFromOwner();
	bool AreAbilitySpecsReady(
		const UPushAbilitySystemComponent* AbilitySystemComponent,
		const TArray<FPushInputActivatedAbilityDisplayData>& Abilities) const;
	void StartAbilitySystemBindingRetry();
	void StopAbilitySystemBindingRetry();

	UPROPERTY(meta = (BindWidget))
	UValueGauge* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UValueGauge* ManaBar;

	UPROPERTY(meta = (BindWidget))
	UAbilityListView* AbilityList;

	UPROPERTY()
	UPushAbilitySystemComponent* OwnerAbilitySystemComponent;

	FTimerHandle AbilitySystemBindingRetryTimerHandle;
	bool bAbilityListConfigured = false;
};
