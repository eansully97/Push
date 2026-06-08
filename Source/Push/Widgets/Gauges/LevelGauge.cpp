// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"

void ULevelGauge::NativeConstruct()
{
	Super::NativeConstruct();

	ClearDelegates();
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	APawn* OwnerPawn = GetOwningPlayerPawn();
	if (!OwnerPawn)
		return;

	UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!OwnerAbilitySystemComponent)
		return;

	OwnerASC = OwnerAbilitySystemComponent;
	UpdateGauge(FOnAttributeChangeData());
	ExperienceChangedDelegateHandle =
		OwnerAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetExperienceAttribute())
		.AddUObject(this, &ThisClass::UpdateGauge);
	NextLevelExperienceChangedDelegateHandle =
		OwnerAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetNextLevelExperienceAttribute())
		.AddUObject(this, &ThisClass::UpdateGauge);
	PrevLevelExperienceChangedDelegateHandle =
		OwnerAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetPrevLevelExperienceAttribute())
		.AddUObject(this, &ThisClass::UpdateGauge);
	LevelChangedDelegateHandle =
		OwnerAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetLevelAttribute())
		.AddUObject(this, &ThisClass::UpdateGauge);
}

void ULevelGauge::NativeDestruct()
{
	ClearDelegates();
	Super::NativeDestruct();
}

void ULevelGauge::ClearDelegates()
{
	if (!OwnerASC)
	{
		return;
	}

	if (ExperienceChangedDelegateHandle.IsValid())
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetExperienceAttribute())
			.Remove(ExperienceChangedDelegateHandle);
		ExperienceChangedDelegateHandle.Reset();
	}

	if (NextLevelExperienceChangedDelegateHandle.IsValid())
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetNextLevelExperienceAttribute())
			.Remove(NextLevelExperienceChangedDelegateHandle);
		NextLevelExperienceChangedDelegateHandle.Reset();
	}

	if (PrevLevelExperienceChangedDelegateHandle.IsValid())
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetPrevLevelExperienceAttribute())
			.Remove(PrevLevelExperienceChangedDelegateHandle);
		PrevLevelExperienceChangedDelegateHandle.Reset();
	}

	if (LevelChangedDelegateHandle.IsValid())
	{
		OwnerASC->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetLevelAttribute())
			.Remove(LevelChangedDelegateHandle);
		LevelChangedDelegateHandle.Reset();
	}

	OwnerASC = nullptr;
}

void ULevelGauge::UpdateGauge(const FOnAttributeChangeData& Data)
{
	if (!OwnerASC)
	{
		return;
	}

	bool bFound = false;
	float CurrentXP = OwnerASC->GetGameplayAttributeValue(UPushHeroAttributeSet::GetExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float NextLevelXP = OwnerASC->GetGameplayAttributeValue(UPushHeroAttributeSet::GetNextLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float PrevLevXP = OwnerASC->GetGameplayAttributeValue(UPushHeroAttributeSet::GetPrevLevelExperienceAttribute(), bFound);
	if (!bFound)
		return;
	float CurrentLevel = OwnerASC->GetGameplayAttributeValue(UPushHeroAttributeSet::GetLevelAttribute(), bFound);
	if (!bFound)
		return;

	LevelText->SetText(FText::AsNumber(CurrentLevel, &NumberFormattingOptions));

	float Progress = CurrentXP - PrevLevXP;
	float LevelXPAmount = NextLevelXP - PrevLevXP;

	float PercentXP = LevelXPAmount > 0.f ? Progress / LevelXPAmount : 1.f;

	if (NextLevelXP == 0)
	{
		PercentXP = 1;
	}

	if (LevelProgressImage)
	{
		LevelProgressImage->GetDynamicMaterial()->SetScalarParameterValue(PercentMaterialParamName, PercentXP);
	}
}
