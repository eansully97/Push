// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_GroundBlast.generated.h"

class ATargetActor_GroundPick;
/**
 * 
 */
UCLASS()
class PUSH_API UGA_GroundBlast : public UPushGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_GroundBlast();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle);

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGenericDamageEffectDef DamageEffectDef;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange = 2000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<ATargetActor_GroundPick> TargetActorClass;
};
