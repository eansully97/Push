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
	ACharacter* GetOwningAvatarCharacter();

protected:
	
	UAnimInstance* GetOwnerAnimInstance() const;
	
	TArray<FHitResult> GetHitResultFromSweepLocationTargetData(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		float SphereSweepRadius = 30.f,
		ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile,
		bool bDrawDebug = false,
		bool bIgnoreSelf = true) const;

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }

	void PushSelf(const FVector& PushVelocity);
	void PushTarget(AActor* Target, const FVector& PushVelocity);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	ACharacter* AvatarCharacter;
};
