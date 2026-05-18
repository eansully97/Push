// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemStatics.h"

FGameplayTag UPushAbilitySystemStatics::GetBasicAttackAbilityTag()
{
	return FGameplayTag::RequestGameplayTag("Ability.BasicAttack");
}
