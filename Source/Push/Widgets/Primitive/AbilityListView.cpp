// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityListView.h"

#include "Abilities/GameplayAbility.h"

void UAbilityListView::ConfigureAbilities(const TArray<FPushInputActivatedAbilityDisplayData>& Abilities)
{
	ClearListItems();
	OnEntryWidgetGenerated().RemoveAll(this);
	OnEntryWidgetGenerated().AddUObject(this, &ThisClass::AbilityGaugeGenerated);
	for (const FPushInputActivatedAbilityDisplayData& AbilityData : Abilities)
	{
		if (AbilityData.AbilityClass)
		{
			UAbilityListItem* AbilityListItem = NewObject<UAbilityListItem>(this);
			AbilityListItem->Initialize(AbilityData);
			AddItem(AbilityListItem);
		}
	}
}

void UAbilityListView::AbilityGaugeGenerated(UUserWidget& Widget)
{
	if (UAbilityGauge* AbilityGauge = Cast<UAbilityGauge>(&Widget))
	{
		if (const UAbilityListItem* AbilityListItem = AbilityGauge->GetListItem<UAbilityListItem>())
		{
			AbilityGauge->ConfigureWithWidgetData(FindWidgetDataForAbility(AbilityListItem->GetAbilityClass()));
		}
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
