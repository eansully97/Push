// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_LevelBased.h"

#include "Push/GAS/Attributes/PushHeroAttributeSet.h"

UMMC_LevelBased::UMMC_LevelBased()
{
	LevelCaptureDefinition.AttributeToCapture = UPushHeroAttributeSet::GetLevelAttribute();
	LevelCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(LevelCaptureDefinition);
}

float UMMC_LevelBased::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	UAbilitySystemComponent* ASC = Spec.GetContext().GetInstigatorAbilitySystemComponent();
	if (!ASC)
		return 0.0f;

	FAggregatorEvaluateParameters EvaluateParams;
	EvaluateParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	float Level = 0.0f;
	GetCapturedAttributeMagnitude(LevelCaptureDefinition, Spec, EvaluateParams, Level);

	bool bFound = false;
	float RateAttributeValue = ASC->GetGameplayAttributeValue(RateAttribute, bFound);
	if (!bFound)
		return 0.0f;

	return (Level - 1) * RateAttributeValue;
}
