// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "AbilityGauge.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class PUSH_API UAbilityListItem : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FPushInputActivatedAbilityDisplayData& InAbilityData);

	EAbilityInputID GetInputID() const;
	TSubclassOf<UGameplayAbility> GetAbilityClass() const;
	UGameplayAbility* GetAbilityDefaultObject() const;

private:
	UPROPERTY()
	EAbilityInputID InputID = EAbilityInputID::None;

	UPROPERTY()
	TSubclassOf<UGameplayAbility> AbilityClass;
};

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};

/**
 * 
 */
UCLASS()
class PUSH_API UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* AbilityWidgetData);
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);

private:
	UPROPERTY(meta = (BindWidget))
	UImage* Icon;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName IconMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CooldownPercentParamName = "Percent";

	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdateInterval = 0.1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownCounterText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CooldownDurationText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CostText;

	UPROPERTY()
	UGameplayAbility* AbilityObject;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY()
	float CachedCooldownDuration;

	UPROPERTY()
	float CachedCooldownTimeRemaining;

	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDecimalFormattingOptions;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownUpdateTimerHandle;
	TMap<FGameplayTag, FDelegateHandle> CooldownTagDelegateHandles;

	void ClearCooldownTimers();
	void SetIconTexture(UTexture2D* IconTexture) const;
	void BindCooldownTagEvents();
	void ClearCooldownTagEvents();
	void CooldownTagChanged(const FGameplayTag Tag, int32 Count);
	void RefreshCooldownState();
	bool GetCooldownTimeRemainingAndDuration(float& CooldownTimeRemaining, float& CooldownDuration) const;
	void CooldownFinished();
	void UpdateCooldown();
};
