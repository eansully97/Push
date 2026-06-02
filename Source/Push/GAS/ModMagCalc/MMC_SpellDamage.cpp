// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_SpellDamage.h"

#include "Push/GAS/Attributes/PushAttributeSet.h"

UMMC_SpellDamage::UMMC_SpellDamage()
{
	DamageCaptureDefinition.AttributeToCapture = UPushAttributeSet::GetSpellPowerAttribute();
	DamageCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	ResistCaptureDefinition.AttributeToCapture = UPushAttributeSet::GetSpellResistAttribute();
	ResistCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(DamageCaptureDefinition);
	RelevantAttributesToCapture.Add(ResistCaptureDefinition);
}

float UMMC_SpellDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float SpellPower = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDefinition, Spec, EvaluateParameters, SpellPower);

	float SpellResist = 0.f;
	GetCapturedAttributeMagnitude(ResistCaptureDefinition, Spec, EvaluateParameters, SpellResist);

	float Damage = SpellPower * ( 1 - SpellResist / (SpellResist + 100));
	return -Damage;
}
