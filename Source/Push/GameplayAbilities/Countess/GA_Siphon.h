// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_Siphon.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UGA_Siphon : public UPushGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Siphon();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float DamageRadius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage", meta = (ClampMin = "0.01"))
	float DamageInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Cost", meta = (ClampMin = "0.01"))
	float CostInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Healing", meta = (ClampMin = "0.0"))
	float HealRatio = 0.25f;

	UFUNCTION()
	void HandleInputReleased(float TimeHeld);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageCancelled();

	void StartMontageLoop();
	void StartSiphonTimers();
	void StopSiphonTimers();
	void DisableAvatarMovement();
	void RestoreAvatarMovement();
	void PerformDamageTick();
	void PerformCostTick();
	bool HasValidSiphonConfig() const;
	bool IsValidSiphonTarget(AActor* TargetActor) const;
	float DamageTargetAndGetHealAmount(AActor* TargetActor);
	void HealOwner(float HealAmount) const;

	FTimerHandle DamageTimerHandle;
	FTimerHandle CostTimerHandle;
	bool bEndingAbility = false;
	bool bCommitCooldownOnEnd = false;
	bool bMovementDisabledByAbility = false;
	uint8 PreviousMovementMode = 0;
	uint8 PreviousCustomMovementMode = 0;
};
