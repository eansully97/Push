// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MinionBarracks.generated.h"

class AMinion;

UCLASS()
class PUSH_API AMinionBarracks : public AActor
{
	GENERATED_BODY()

public:

protected:
	virtual void BeginPlay() override;

private:
	void SpawnNewMinions(int32 AmountToSpawn);
	void SpawnNewGroup();
	const APlayerStart* GetNextSpawnStart();
	AMinion* GetNextAvailableMinion() const;
	
	
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarracksTeamID;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MinionPerGroup = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 5.f;

	UPROPERTY()
	TArray<AMinion*> MinionPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<AMinion> MinionClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<APlayerStart*> MinionPlayerStarts;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	AActor* Goal;

	int32 NextSpawnSpotIndex = -1;
	FTimerHandle SpawnIntervalTimerHandle;
};
