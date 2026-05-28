// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityGauge.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "GameplayEffect.h"
#include "Push/GAS/PushAbilitySystemStatics.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UAbilityListItem::Initialize(const FPushInputActivatedAbilityDisplayData& InAbilityData)
{
	InputID = InAbilityData.InputID;
	AbilityClass = InAbilityData.AbilityClass;
}

EAbilityInputID UAbilityListItem::GetInputID() const
{
	return InputID;
}

TSubclassOf<UGameplayAbility> UAbilityListItem::GetAbilityClass() const
{
	return AbilityClass;
}

UGameplayAbility* UAbilityListItem::GetAbilityDefaultObject() const
{
	return AbilityClass ? AbilityClass.GetDefaultObject() : nullptr;
}

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	WholeNumberFormattingOptions.SetMinimumFractionalDigits(0);
	WholeNumberFormattingOptions.SetMaximumFractionalDigits(0);
	TwoDecimalFormattingOptions.SetMinimumFractionalDigits(1);
	TwoDecimalFormattingOptions.SetMaximumFractionalDigits(1);
}

void UAbilityGauge::NativeDestruct()
{
	ClearCooldownTagEvents();
	ClearCooldownTimers();
	Super::NativeDestruct();
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	ClearCooldownTagEvents();
	const UAbilityListItem* AbilityListItem = Cast<UAbilityListItem>(ListItemObject);
	AbilityObject = AbilityListItem ? AbilityListItem->GetAbilityDefaultObject() : Cast<UGameplayAbility>(ListItemObject);

	float CooldownDuration = UPushAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityObject);
	int32 Cost = UPushAbilitySystemStatics::GetStaticCostForAbility(AbilityObject);

	CostText->SetText(FText::AsNumber(Cost));
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration, &WholeNumberFormattingOptions));
	BindCooldownTagEvents();
	RefreshCooldownState();
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* AbilityWidgetData)
{
	if (!Icon || !AbilityWidgetData || AbilityWidgetData->Icon.IsNull())
	{
		return;
	}

	if (UTexture2D* LoadedIcon = AbilityWidgetData->Icon.Get())
	{
		SetIconTexture(LoadedIcon);
		return;
	}

	AbilityWidgetData->Icon.LoadAsync(
		FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(
			this,
			[this](const FSoftObjectPath& Path, UObject* LoadedObject)
			{
				SetIconTexture(Cast<UTexture2D>(LoadedObject));
			}));
}

void UAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	if (CooldownTimeRemaining <= 0.f || CooldownDuration <= 0.f)
	{
		CooldownFinished();
		return;
	}

	ClearCooldownTimers();

	CooldownCounterText->SetVisibility(ESlateVisibility::Visible);
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration, &WholeNumberFormattingOptions));
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle,this, &ThisClass::CooldownFinished, CooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle,this, &ThisClass::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UAbilityGauge::BindCooldownTagEvents()
{
	if (!OwnerAbilitySystemComponent)
	{
		OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	}

	if (!OwnerAbilitySystemComponent || !AbilityObject)
	{
		return;
	}

	const FGameplayTagContainer* CooldownTags = AbilityObject->GetCooldownTags();
	if (!CooldownTags)
	{
		return;
	}

	for (const FGameplayTag& CooldownTag : *CooldownTags)
	{
		FDelegateHandle DelegateHandle = OwnerAbilitySystemComponent
			->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::AnyCountChange)
			.AddUObject(this, &ThisClass::CooldownTagChanged);

		CooldownTagDelegateHandles.Add(CooldownTag, DelegateHandle);
	}
}

void UAbilityGauge::ClearCooldownTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
		World->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
	}
}

void UAbilityGauge::SetIconTexture(UTexture2D* IconTexture) const
{
	if (Icon && IconTexture)
	{
		Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, IconTexture);
	}
}

void UAbilityGauge::ClearCooldownTagEvents()
{
	if (OwnerAbilitySystemComponent)
	{
		for (const TPair<FGameplayTag, FDelegateHandle>& DelegateHandlePair : CooldownTagDelegateHandles)
		{
			OwnerAbilitySystemComponent->RegisterGameplayTagEvent(
				DelegateHandlePair.Key,
				EGameplayTagEventType::AnyCountChange
			).Remove(DelegateHandlePair.Value);
		}
	}

	CooldownTagDelegateHandles.Empty();
}

void UAbilityGauge::CooldownTagChanged(const FGameplayTag Tag, int32 Count)
{
	RefreshCooldownState();
}

void UAbilityGauge::RefreshCooldownState()
{
	float CooldownTimeRemaining = 0.f;
	float CooldownDuration = 0.f;
	if (GetCooldownTimeRemainingAndDuration(CooldownTimeRemaining, CooldownDuration))
	{
		StartCooldown(CooldownTimeRemaining, CooldownDuration);
	}
	else
	{
		CooldownFinished();
	}
}

bool UAbilityGauge::GetCooldownTimeRemainingAndDuration(float& CooldownTimeRemaining, float& CooldownDuration) const
{
	CooldownTimeRemaining = 0.f;
	CooldownDuration = 0.f;

	if (!OwnerAbilitySystemComponent || !AbilityObject)
	{
		return false;
	}

	const FGameplayTagContainer* CooldownTags = AbilityObject->GetCooldownTags();
	if (!CooldownTags || CooldownTags->Num() == 0)
	{
		return false;
	}

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(*CooldownTags);
	TArray<TPair<float, float>> CooldownDurations =
		OwnerAbilitySystemComponent->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	if (CooldownDurations.IsEmpty())
	{
		return false;
	}

	int32 BestCooldownIndex = 0;
	float LongestTimeRemaining = CooldownDurations[0].Key;
	for (int32 Index = 1; Index < CooldownDurations.Num(); ++Index)
	{
		if (CooldownDurations[Index].Key > LongestTimeRemaining)
		{
			LongestTimeRemaining = CooldownDurations[Index].Key;
			BestCooldownIndex = Index;
		}
	}

	CooldownTimeRemaining = CooldownDurations[BestCooldownIndex].Key;
	CooldownDuration = CooldownDurations[BestCooldownIndex].Value;
	return CooldownTimeRemaining > 0.f && CooldownDuration > 0.f;
}

void UAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	ClearCooldownTimers();
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f);
}

void UAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;
	if (CachedCooldownTimeRemaining <= 0.f || CachedCooldownDuration <= 0.f)
	{
		CooldownFinished();
		return;
	}

	const FNumberFormattingOptions* FormattingOptions =
		CachedCooldownTimeRemaining > 1.f ? &WholeNumberFormattingOptions : &TwoDecimalFormattingOptions;
	CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));

	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}
