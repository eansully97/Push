// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GroundPick.generated.h"

UCLASS()
class PUSH_API ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	ATargetActor_GroundPick();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void ConfirmTargetingAndContinue() override;
	
	FVector GetTargetPoint() const;

	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy = true);

	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	bool bShouldDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;

public:
	FORCEINLINE void SetShouldDrawDebug(const bool bDrawDebug) { bShouldDrawDebug = bDrawDebug; }
	FORCEINLINE void SetTargetAreaRadius(const float NewRadius) { TargetAreaRadius = NewRadius; }
	FORCEINLINE void SetTargetTraceRange(const float NewTraceRange) { TargetTraceRange = NewTraceRange; }
};
