// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_GroundBlast.generated.h"

class ATargetActor_GroundPick;
class APlayerController;
class UCameraShakeBase;
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

	bool HasValidGroundBlastConfig() const;
	bool TryGetValidatedTargetLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FVector& OutTargetLocation) const;
	FGameplayAbilityTargetDataHandle BuildServerTargetData(const FVector& TargetLocation) const;
	APlayerController* GetLocalPlayerController() const;
	void PlayLocalCameraShake() const;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGenericDamageEffectDef DamageEffectDef;

	UPROPERTY(EditDefaultsOnly, Category = "Cue")
	FGameplayTag GameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange = 2000.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* TargetingAbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* CastAbilityMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<ATargetActor_GroundPick> TargetActorClass;
};
