// Fill out your copyright notice in the Description page of Project Settings.


#include "MMC_AttackDamage.h"

#include "Push/GAS/Attributes/PushAttributeSet.h"

UMMC_AttackDamage::UMMC_AttackDamage()
{
	DamageCaptureDefinition.AttributeToCapture = UPushAttributeSet::GetAttackDamageAttribute();
	DamageCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	ArmorCaptureDefinition.AttributeToCapture = UPushAttributeSet::GetArmorAttribute();
	ArmorCaptureDefinition.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(DamageCaptureDefinition);
	RelevantAttributesToCapture.Add(ArmorCaptureDefinition);
}

float UMMC_AttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float AttackDamage = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDefinition, Spec, EvaluateParameters, AttackDamage);

	float Armor = 0.f;
	GetCapturedAttributeMagnitude(ArmorCaptureDefinition, Spec, EvaluateParameters, Armor);

	float Damage = AttackDamage * ( 1 - Armor / (Armor + 100));
	return -Damage;
}
