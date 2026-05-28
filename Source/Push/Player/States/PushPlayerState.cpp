// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerState.h"

#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"

APushPlayerState::APushPlayerState()
{
	SetNetUpdateFrequency(100.f);

	PushAbilitySystemComponent = CreateDefaultSubobject<UPushAbilitySystemComponent>("PushAbilitySystemComponent");
	PushAbilitySystemComponent->SetIsReplicated(true);
	PushAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	PushAttributeSet = CreateDefaultSubobject<UPushAttributeSet>("PushAttributeSet");
}

UAbilitySystemComponent* APushPlayerState::GetAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}

UPushAbilitySystemComponent* APushPlayerState::GetPushAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}

UPushAttributeSet* APushPlayerState::GetPushAttributeSet() const
{
	return PushAttributeSet;
}
