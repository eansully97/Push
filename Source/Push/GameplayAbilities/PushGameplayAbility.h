// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "PushGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UPushGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius = 30.f, bool bDrawDebug = false, bool bIgnoreSelf = true) const;
};
