// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Push/Character/Base/PushCharacter.h"
#include "Minion.generated.h"

UCLASS()
class PUSH_API AMinion : public APushCharacter
{
	GENERATED_BODY()
	
public:
	AMinion();

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	bool IsActive() const;
	void Activate();
	void Activate(const FTransform& SpawnTransform);
	void SetGoal(AActor* NewGoal);

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual bool ShouldApplyInitialEffects() const override;
	
private:
	void PickMeshForTeamID();
	void ApplyGoalToBlackboard() const;

	virtual void OnRep_TeamID() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TMap<FGenericTeamId, USkeletalMesh*> TeamMeshMap;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName GoalBlackboardKeyName = "Goal";

	UPROPERTY()
	TObjectPtr<AActor> Goal;
};
