// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PushAbilitySystemStatics.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UPushAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FGameplayTag GetBasicAttackAbilityTag();
};
