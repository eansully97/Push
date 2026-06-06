// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PushAbilitySystemStatics.generated.h"

class UGameplayAbility;
/**
 * 
 */
UCLASS()
class PUSH_API UPushAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);
	static bool IsHero(const AActor* ActorToCheck);
	static bool IsAbilityMaxLevel(const FGameplayAbilitySpec& AbilitySpec);
	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
	static float GetManaCostFor(const UGameplayAbility* AbilityObj, const UAbilitySystemComponent& ASC, int32 AbilityLevel);
	static float GetCooldownDurationFor(const UGameplayAbility* AbilityObj, const UAbilitySystemComponent& ASC, int32 AbilityLevel);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityObj, const UAbilitySystemComponent& ASC);
};
