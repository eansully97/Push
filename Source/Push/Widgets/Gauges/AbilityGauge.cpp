// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityGauge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Components/Image.h"
#include "GameplayEffect.h"
#include "Push/GAS/PushAbilitySystemStatics.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "TimerManager.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"

void UAbilityListItem::Initialize(
	const FPushInputActivatedAbilityDisplayData& InAbilityData,
	UPushAbilitySystemComponent* InAbilitySystemComponent)
{
	InputID = InAbilityData.InputID;
	AbilityClass = InAbilityData.AbilityClass;
	AbilitySystemComponent = InAbilitySystemComponent;
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

UPushAbilitySystemComponent* UAbilityListItem::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void UAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();

	WholeNumberFormattingOptions.SetMinimumFractionalDigits(0);
	WholeNumberFormattingOptions.SetMaximumFractionalDigits(0);
	TwoDecimalFormattingOptions.SetMinimumFractionalDigits(1);
	TwoDecimalFormattingOptions.SetMaximumFractionalDigits(1);

	ResetVisualState();
}

void UAbilityGauge::NativeDestruct()
{
	ClearAbilityState();
	Super::NativeDestruct();
}

void UAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	ClearAbilityState();
	ResetVisualState();

	const UAbilityListItem* AbilityListItem = Cast<UAbilityListItem>(ListItemObject);
	if (!AbilityListItem)
	{
		return;
	}

	InputID = AbilityListItem->GetInputID();
	AbilityObject = AbilityListItem->GetAbilityDefaultObject();
	OwnerAbilitySystemComponent = AbilityListItem->GetAbilitySystemComponent();

	float CooldownDuration = UPushAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityObject);
	int32 Cost = UPushAbilitySystemStatics::GetStaticCostForAbility(AbilityObject);

	CostText->SetText(FText::AsNumber(Cost));
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration, &WholeNumberFormattingOptions));
	BindAbilityState();
	RefreshFromCurrentState();
}

void UAbilityGauge::NativeOnEntryReleased()
{
	ClearAbilityState();
	ResetVisualState();
	IUserListEntry::NativeOnEntryReleased();
}

void UAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* AbilityWidgetData)
{
	if (!Icon || !AbilityObject || !AbilityWidgetData || AbilityWidgetData->AbilityClass != AbilityObject->GetClass()
		|| AbilityWidgetData->Icon.IsNull())
	{
		return;
	}

	if (UTexture2D* LoadedIcon = AbilityWidgetData->Icon.Get())
	{
		SetIconTexture(LoadedIcon);
		return;
	}

	const TSubclassOf<UGameplayAbility> ExpectedAbilityClass = AbilityWidgetData->AbilityClass;
	AbilityWidgetData->Icon.LoadAsync(
		FLoadSoftObjectPathAsyncDelegate::CreateWeakLambda(
			this,
			[this, ExpectedAbilityClass](const FSoftObjectPath& Path, UObject* LoadedObject)
			{
				if (AbilityObject && AbilityObject->GetClass() == ExpectedAbilityClass)
				{
					SetIconTexture(Cast<UTexture2D>(LoadedObject));
				}
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
	if (!OwnerAbilitySystemComponent || !AbilityObject || !CooldownTagDelegateHandles.IsEmpty())
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

const FGameplayAbilitySpec* UAbilityGauge::GetAbilitySpec() const
{
	if (!OwnerAbilitySystemComponent || InputID == EAbilityInputID::None)
	{
		return nullptr;
	}

	return OwnerAbilitySystemComponent->FindAbilitySpecFromInputID(static_cast<int32>(InputID));
}

void UAbilityGauge::BindAbilityState()
{
	if (!OwnerAbilitySystemComponent || !AbilityObject)
	{
		return;
	}

	AbilitySpecDirtiedDelegateHandle =
		OwnerAbilitySystemComponent->AbilitySpecDirtiedCallbacks.AddUObject(this, &ThisClass::AbilitySpecUpdated);
	UpgradePointChangedDelegateHandle =
		OwnerAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetUpgradePointAttribute())
			.AddUObject(this, &ThisClass::UpgradePointUpdated);
	ManaCostChangedDelegateHandle =
		OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetManaAttribute())
		.AddUObject(this, &ThisClass::ManaUpdated);
	BindCooldownTagEvents();
}

void UAbilityGauge::ClearAbilityState()
{
	ClearCooldownTimers();
	ClearCooldownTagEvents();

	if (OwnerAbilitySystemComponent)
	{
		if (AbilitySpecDirtiedDelegateHandle.IsValid())
		{
			OwnerAbilitySystemComponent->AbilitySpecDirtiedCallbacks.Remove(AbilitySpecDirtiedDelegateHandle);
		}
		if (UpgradePointChangedDelegateHandle.IsValid())
		{
			OwnerAbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetUpgradePointAttribute())
				.Remove(UpgradePointChangedDelegateHandle);
		}
		if (ManaCostChangedDelegateHandle.IsValid())
		{
			OwnerAbilitySystemComponent
				->GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetManaAttribute())
				.Remove(ManaCostChangedDelegateHandle);
		}
	}

	AbilitySpecDirtiedDelegateHandle.Reset();
	UpgradePointChangedDelegateHandle.Reset();
	ManaCostChangedDelegateHandle.Reset();
	OwnerAbilitySystemComponent = nullptr;
	AbilityObject = nullptr;
	InputID = EAbilityInputID::None;
}

void UAbilityGauge::ResetVisualState()
{
	CachedCooldownDuration = 0.f;
	CachedCooldownTimeRemaining = 0.f;

	if (CooldownCounterText)
	{
		CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Icon && Icon->GetDynamicMaterial())
	{
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamName, 1.f);
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CanCastParamName, 0.f);
		Icon->GetDynamicMaterial()->SetScalarParameterValue(UpgradePointAvailableName, 0.f);
	}
	if (LevelGauge && LevelGauge->GetDynamicMaterial())
	{
		LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParamName, 0.f);
	}
}

void UAbilityGauge::RefreshFromCurrentState()
{
	RefreshAbilitySpecState();
	RefreshUpgradeAvailability();
	RefreshCooldownState();
}

void UAbilityGauge::RefreshAbilitySpecState()
{
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	const int32 AbilityLevel = AbilitySpec ? AbilitySpec->Level : 0;

	if (LevelGauge && LevelGauge->GetDynamicMaterial())
	{
		LevelGauge->GetDynamicMaterial()->SetScalarParameterValue(AbilityLevelParamName, AbilityLevel);
	}

	UpdateCanCast();
}

void UAbilityGauge::UpdateCanCast()
{
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	const bool bCanCast =
		AbilitySpec
		&& AbilitySpec->Level > 0
		&& OwnerAbilitySystemComponent
		&& UPushAbilitySystemStatics::CheckAbilityCost(*AbilitySpec, *OwnerAbilitySystemComponent);

	if (Icon && Icon->GetDynamicMaterial())
	{
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CanCastParamName, bCanCast ? 1.f : 0.f);
	}
}

void UAbilityGauge::RefreshUpgradeAvailability()
{
	bool bFoundUpgradePoints = false;
	const float UpgradePoints = OwnerAbilitySystemComponent
		? OwnerAbilitySystemComponent->GetGameplayAttributeValue(
			UPushHeroAttributeSet::GetUpgradePointAttribute(),
			bFoundUpgradePoints)
		: 0.f;
	const FGameplayAbilitySpec* AbilitySpec = GetAbilitySpec();
	const bool bUpgradeAvailable =
		bFoundUpgradePoints
		&& UpgradePoints > 0.f
		&& AbilitySpec
		&& !UPushAbilitySystemStatics::IsAbilityMaxLevel(*AbilitySpec);

	if (Icon && Icon->GetDynamicMaterial())
	{
		Icon->GetDynamicMaterial()->SetScalarParameterValue(
			UpgradePointAvailableName,
			bUpgradeAvailable ? 1.f : 0.f);
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

void UAbilityGauge::CooldownTagChanged(const FGameplayTag, int32)
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

void UAbilityGauge::AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.InputID != static_cast<int32>(InputID))
		return;

	RefreshAbilitySpecState();
	RefreshUpgradeAvailability();

	float NewCooldownDuration = UPushAbilitySystemStatics::GetCooldownDurationFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);
	float NewCost = UPushAbilitySystemStatics::GetManaCostFor(AbilitySpec.Ability, *OwnerAbilitySystemComponent, AbilitySpec.Level);

	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MaximumFractionalDigits = 0;
	
	CooldownDurationText->SetText(FText::AsNumber(NewCooldownDuration));
	CostText->SetText(FText::AsNumber(NewCost, &FormattingOptions));
}

void UAbilityGauge::UpgradePointUpdated(const FOnAttributeChangeData&)
{
	RefreshUpgradeAvailability();
}

void UAbilityGauge::ManaUpdated(const FOnAttributeChangeData&)
{
	UpdateCanCast();
}
