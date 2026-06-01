// Fill out your copyright notice in the Description page of Project Settings.


#include "Minion.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AMinion::AMinion()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bUseRVOAvoidance = true;
		MovementComponent->AvoidanceConsiderationRadius = 180.f;
		MovementComponent->AvoidanceWeight = 0.75f;
	}
}

void AMinion::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	Super::SetGenericTeamId(NewTeamID);
	PickMeshForTeamID();
}

bool AMinion::IsActive() const
{
	return !IsDead();
}

bool AMinion::ShouldApplyInitialEffects() const
{
	return true;
}

void AMinion::Activate()
{
	RespawnImmediately();
	ApplyGoalToBlackboard();
	
}

void AMinion::Activate(const FTransform& SpawnTransform)
{
	RespawnImmediately();
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	ApplyGoalToBlackboard();
}

void AMinion::SetGoal(AActor* NewGoal)
{
	Goal = NewGoal;
	ApplyGoalToBlackboard();
}

void AMinion::BeginPlay()
{
	Super::BeginPlay();
	ApplyGoalToBlackboard();
}

void AMinion::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	ApplyGoalToBlackboard();
}

void AMinion::ApplyGoalToBlackboard() const
{
	if (AAIController* AIController = GetController<AAIController>())
	{
		if (UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			Blackboard->SetValueAsObject(GoalBlackboardKeyName, Goal);
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
