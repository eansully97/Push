// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/Widgets/Gauges/AbilityGauge.h"
#include "AbilityListView.generated.h"

class UPushAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PUSH_API UAbilityListView : public UListView
{
	GENERATED_BODY()
public:
	void ConfigureAbilities(UPushAbilitySystemComponent* AbilitySystemComponent);
	void ConfigureAbilities(
		UPushAbilitySystemComponent* AbilitySystemComponent,
		const TArray<FPushInputActivatedAbilityDisplayData>& Abilities);
	void AbilityGaugeGenerated(UUserWidget& Widget);

	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* AbilityDataTable;

	const FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;
};
