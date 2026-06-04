// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "PushGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UPushGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPushGameplayAbility();
	ACharacter* GetOwningAvatarCharacter();

protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		float SphereSweepRadius = 30.f,
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		bool bDrawDebug = false,
		bool bIgnoreSelf = true) const;

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }

	void ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> Effect, int32 Level = 1);
	void PushSelf(const FVector& PushVelocity);
	void PushTarget(AActor* Target, const FVector& PushVelocity);
	void PushTargets(TArray<AActor*>& Targets, const FVector& PushVelocity);
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVelocity);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};
