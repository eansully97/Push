// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	static int32 GetStaticCostForAbility(const UGameplayAbility* Ability);
	static bool IsHero(const AActor* ActorToCheck);
};
