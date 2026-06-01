// Fill out your copyright notice in the Description page of Project Settings.


#include "MinionBarracks.h"

#include "Components/CapsuleComponent.h"
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
	if (!MinionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s cannot spawn minions: MinionClass is not set."), *GetName());
		return;
	}

	for (int32 i = 0; i < AmountToSpawn; i++)
	{
		FTransform SpawnTransform = GetNextBaseSpawnTransform();
		if (!TryFindClearSpawnTransform(nullptr, SpawnTransform))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s skipped spawning %s because no clear minion spawn spot was available."),
				*GetName(),
				*GetNameSafe(MinionClass));
			continue;
		}
		
		AMinion* NewMinion = GetWorld()->SpawnActorDeferred<AMinion>(MinionClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
		if (!NewMinion)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s failed to spawn minion class %s."),
				*GetName(),
				*MinionClass->GetName());
			continue;
		}

		NewMinion->SetGenericTeamId(BarracksTeamID);
		NewMinion->SetGoal(Goal);
		NewMinion->FinishSpawning(SpawnTransform);
		
		MinionPool.Add(NewMinion);
	}
}

void AMinionBarracks::SpawnNewGroup()
{
	int32 i = MinionPerGroup;

	while (i >0)
	{
		FTransform SpawnTransform = GetNextBaseSpawnTransform();

		AMinion* NextAvailableMinion = GetNextAvailableMinion();
		if (!NextAvailableMinion)
			break;

		if (!TryFindClearSpawnTransform(NextAvailableMinion, SpawnTransform))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s skipped respawning %s because no clear minion spawn spot was available."),
				*GetName(),
				*GetNameSafe(NextAvailableMinion));
			break;
		}

		NextAvailableMinion->Activate(SpawnTransform);
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

FTransform AMinionBarracks::GetNextBaseSpawnTransform()
{
	if (const APlayerStart* NextSpawnSpot = GetNextSpawnStart())
	{
		return NextSpawnSpot->GetActorTransform();
	}

	return GetActorTransform();
}

bool AMinionBarracks::TryFindClearSpawnTransform(const AMinion* MinionToPlace, FTransform& InOutSpawnTransform) const
{
	if (IsSpawnTransformClear(MinionToPlace, InOutSpawnTransform))
	{
		return true;
	}

	const int32 SlotsPerRing = FMath::Max(1, SpawnSearchSlotsPerRing);
	for (int32 RingIndex = 1; RingIndex <= SpawnSearchRings; ++RingIndex)
	{
		const float Radius = SpawnSearchRingSpacing * RingIndex;
		for (int32 SlotIndex = 0; SlotIndex < SlotsPerRing; ++SlotIndex)
		{
			const float Angle = (2.f * UE_PI * SlotIndex) / SlotsPerRing;
			FTransform CandidateTransform = InOutSpawnTransform;
			CandidateTransform.AddToTranslation(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f));

			if (IsSpawnTransformClear(MinionToPlace, CandidateTransform))
			{
				InOutSpawnTransform = CandidateTransform;
				return true;
			}
		}
	}

	return false;
}

bool AMinionBarracks::IsSpawnTransformClear(const AMinion* MinionToPlace, const FTransform& SpawnTransform) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const AMinion* ProbeMinion = MinionToPlace;
	if (!ProbeMinion && MinionClass)
	{
		ProbeMinion = MinionClass->GetDefaultObject<AMinion>();
	}

	const UCapsuleComponent* CapsuleComponent = ProbeMinion ? ProbeMinion->GetCapsuleComponent() : nullptr;
	if (!CapsuleComponent)
	{
		return true;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MinionSpawnClearance), false, this);
	if (MinionToPlace)
	{
		QueryParams.AddIgnoredActor(MinionToPlace);
	}

	for (const AMinion* Minion : MinionPool)
	{
		if (Minion && !Minion->IsActive())
		{
			QueryParams.AddIgnoredActor(Minion);
		}
	}

	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		CapsuleComponent->GetScaledCapsuleRadius(),
		CapsuleComponent->GetScaledCapsuleHalfHeight());

	return !World->OverlapBlockingTestByChannel(
		SpawnTransform.GetLocation(),
		SpawnTransform.GetRotation(),
		CapsuleComponent->GetCollisionObjectType(),
		CapsuleShape,
		QueryParams);
}

AMinion* AMinionBarracks::GetNextAvailableMinion() const
{
	for (AMinion* Minion : MinionPool)
	{
		if (Minion && !Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}
