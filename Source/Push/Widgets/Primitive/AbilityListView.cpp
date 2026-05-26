// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityListView.h"

#include "Abilities/GameplayAbility.h"

void UAbilityListView::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& Abilities)
{
	OnEntryWidgetGenerated().AddUObject(this, &ThisClass::AbilityGaugeGenerated);
	for (const auto& AbilityPair : Abilities)
	{
		AddItem(AbilityPair.Value.GetDefaultObject());
	}
}

void UAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	if (UAbilityGauge* AbilityGauge = Cast<UAbilityGauge>(&Widget))
	{
		AbilityGauge->ConfigureWithWidgetData(FindWidgetDataForAbility(AbilityGauge->GetListItem<UGameplayAbility>()->GetClass()));
	}
}

const FAbilityWidgetData* UAbilityListView::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable)
		return nullptr;

	for (auto& WidgetDataPair : AbilityDataTable->GetRowMap())
	{
		const FAbilityWidgetData* WidgetData = AbilityDataTable->FindRow<FAbilityWidgetData>(WidgetDataPair.Key, "");
		if (WidgetData->AbilityClass == AbilityClass)
		{
			return WidgetData;
		}
	}
	return nullptr;
}
