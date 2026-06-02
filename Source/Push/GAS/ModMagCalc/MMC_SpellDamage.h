// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_SpellDamage.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UMMC_SpellDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
public:
	UMMC_SpellDamage();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition DamageCaptureDefinition;
	FGameplayEffectAttributeCaptureDefinition ResistCaptureDefinition;
};
