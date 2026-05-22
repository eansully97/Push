// Fill out your copyright notice in the Description page of Project Settings.


#include "MinionBarracks.h"

#include "Minion.h"
#include "GameFramework/PlayerStart.h"

void AMinionBarracks::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnIntervalTimerHandle, this, &ThisClass::SpawnNewGroup, SpawnInterval, true);
	}
}

void AMinionBarracks::SpawnNewMinions(int32 AmountToSpawn)
{
	for (int32 i = 0; i < AmountToSpawn; i++)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnStart())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform();
		}
		
		AMinion* NewMinion = GetWorld()->SpawnActorDeferred<AMinion>(MinionClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewMinion->SetGenericTeamId(BarracksTeamID);
		NewMinion->FinishSpawning(SpawnTransform);
		NewMinion->SetGoal(Goal);
		
		MinionPool.Add(NewMinion);
	}
}

void AMinionBarracks::SpawnNewGroup()
{
	int32 i = MinionPerGroup;

	while (i >0)
	{
		FTransform SpawnTransform = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnStart())
		{
			SpawnTransform = NextSpawnSpot->GetActorTransform();
		}

		AMinion* NextAvailableMinion = GetNextAvailableMinion();
		if (!NextAvailableMinion)
			break;

		NextAvailableMinion->SetActorTransform(SpawnTransform);
		NextAvailableMinion->Activate();
		--i;
	}

	SpawnNewMinions(i);
}

const APlayerStart* AMinionBarracks::GetNextSpawnStart()
{
	if (MinionPlayerStarts.Num() == 0)
		return nullptr;

	++NextSpawnSpotIndex;

	if (NextSpawnSpotIndex >= MinionPlayerStarts.Num())
	{
		NextSpawnSpotIndex = 0;
	}
	return MinionPlayerStarts[NextSpawnSpotIndex];
}

AMinion* AMinionBarracks::GetNextAvailableMinion() const
{
	for (AMinion* Minion : MinionPool)
	{
		if (!Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}
