// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "LevelGauge.generated.h"

class UAbilitySystemComponent;
class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class PUSH_API ULevelGauge : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void UpdateGauge(const FOnAttributeChangeData& Data);

private:
	void ClearDelegates();

	UPROPERTY(EditAnywhere, Category = "Visual")
	FName PercentMaterialParamName = "Percent";

	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	FDelegateHandle ExperienceChangedDelegateHandle;
	FDelegateHandle NextLevelExperienceChangedDelegateHandle;
	FDelegateHandle PrevLevelExperienceChangedDelegateHandle;
	FDelegateHandle LevelChangedDelegateHandle;
	
	UPROPERTY(meta = (BindWidget))
	UImage* LevelProgressImage;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelText;

	FNumberFormattingOptions NumberFormattingOptions;
};
