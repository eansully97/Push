// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Actor.h"
#include "MinionBarracks.generated.h"

class AMinion;
class APlayerStart;

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
	FTransform GetNextBaseSpawnTransform();
	bool TryFindClearSpawnTransform(const AMinion* MinionToPlace, FTransform& InOutSpawnTransform) const;
	bool IsSpawnTransformClear(const AMinion* MinionToPlace, const FTransform& SpawnTransform) const;
	AMinion* GetNextAvailableMinion() const;
	
	
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FGenericTeamId BarracksTeamID;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 MinionPerGroup = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 5.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnSearchRingSpacing = 150.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	int32 SpawnSearchRings = 2;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnSearchSlotsPerRing = 8;

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
