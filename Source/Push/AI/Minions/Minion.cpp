// Fill out your copyright notice in the Description page of Project Settings.


#include "Minion.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void AMinion::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	Super::SetGenericTeamId(NewTeamID);
	PickMeshForTeamID();
}

bool AMinion::IsActive() const
{
	return !IsDead();
}

void AMinion::Activate()
{
	RespawnImmediately();
}

void AMinion::SetGoal(AActor* NewGoal) const
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsObject(GoalBlackboardKeyName, NewGoal);
		}
	}
}

void AMinion::PickMeshForTeamID()
{
	if (USkeletalMesh** SkeletalMesh = TeamMeshMap.Find(GetGenericTeamId()))
	{
		GetMesh()->SetSkeletalMesh(*SkeletalMesh);
	}
}

void AMinion::OnRep_TeamID()
{
	PickMeshForTeamID();
}
