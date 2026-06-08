// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetActor_GroundPick.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Components/DecalComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "Push/Push.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root"));
	
	DecalComponent = CreateDefaultSubobject<UDecalComponent>("Decal Component");
	DecalComponent->SetupAttachment(GetRootComponent());
}

void ATargetActor_GroundPick::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SetActorTickEnabled(PrimaryPC && PrimaryPC->IsLocalPlayerController());
}

void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
		RefreshTargetOverlays();
	}
}

void ATargetActor_GroundPick::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearTargetOverlays();

	Super::EndPlay(EndPlayReason);
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(TargetAreaRadius);
	World->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, QueryParams, CollisionShape);
	TSet<AActor*> TargetActors;

	const IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}

	for (const auto& OverlapResult : OverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!IsValidTargetActor(TargetActor, OwnerTeamInterface))
		{
			continue;
		}
		
		TargetActors.Add(TargetActor);
	}

	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	FGameplayAbilityTargetData_SingleTargetHit* HitLocation = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLocation->HitResult.ImpactPoint = GetActorLocation();
	TargetData.Add(HitLocation);
	ClearTargetOverlays();
	TargetDataReadyDelegate.Broadcast(TargetData);
}

FVector ATargetActor_GroundPick::GetTargetPoint() const
{
	if (!PrimaryPC || !PrimaryPC->IsLocalPlayerController())
	{
		return GetActorLocation();
	}
	FHitResult TraceResult;

	FVector ViewLocation;
	FRotator ViewRotation;

	PrimaryPC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector TraceEnd = ViewLocation + ViewRotation.Vector() * TargetTraceRange;
	
	GetWorld()->LineTraceSingleByChannel(TraceResult, ViewLocation, TraceEnd, ECC_Target);

	if (!TraceResult.bBlockingHit)
	{
		GetWorld()->LineTraceSingleByChannel(
			TraceResult,
			TraceEnd,
			TraceEnd + FVector::DownVector * MaxDownwardTraceDistance,
			ECC_Target);
	}

	if (!TraceResult.bBlockingHit)
	{
		return GetActorLocation();
	}

	if (bShouldDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TraceResult.ImpactPoint, TargetAreaRadius, 32, FColor::Cyan);
	}

	return TraceResult.ImpactPoint;
}

void ATargetActor_GroundPick::SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy)
{
	bShouldTargetFriendly = bTargetFriendly;
	bShouldTargetEnemy = bTargetEnemy;
}

void ATargetActor_GroundPick::SetTargetAreaRadius(const float NewRadius)
{
	TargetAreaRadius = NewRadius;
	DecalComponent->DecalSize = FVector(NewRadius);
}

void ATargetActor_GroundPick::RefreshTargetOverlays()
{
	if (!TargetOverlayMaterial)
	{
		ClearTargetOverlays();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		ClearTargetOverlays();
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(TargetAreaRadius);
	World->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, QueryParams, CollisionShape);

	const IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}

	TSet<UMeshComponent*> DesiredOverlayMeshes;
	DesiredOverlayMeshes.Reserve(OverlapResults.Num());
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!IsValidTargetActor(TargetActor, OwnerTeamInterface))
		{
			continue;
		}

		if (UMeshComponent* MeshComponent = GetOverlayMeshComponent(TargetActor))
		{
			DesiredOverlayMeshes.Add(MeshComponent);
			ApplyTargetOverlay(MeshComponent);
		}
	}

	for (int32 Index = OverlayEntries.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponent = OverlayEntries[Index].MeshComponent.Get();
		if (!MeshComponent || !DesiredOverlayMeshes.Contains(MeshComponent))
		{
			RestoreTargetOverlay(Index);
		}
	}
}

void ATargetActor_GroundPick::ClearTargetOverlays()
{
	for (int32 Index = OverlayEntries.Num() - 1; Index >= 0; --Index)
	{
		RestoreTargetOverlay(Index);
	}
}

void ATargetActor_GroundPick::ApplyTargetOverlay(UMeshComponent* MeshComponent)
{
	if (!MeshComponent || HasOverlayEntryFor(MeshComponent))
	{
		return;
	}

	FGroundPickOverlayEntry OverlayEntry;
	OverlayEntry.MeshComponent = MeshComponent;
	OverlayEntry.PreviousOverlayMaterial = MeshComponent->GetOverlayMaterial();
	OverlayEntry.AppliedOverlayMaterial = TargetOverlayMaterial;
	OverlayEntries.Add(OverlayEntry);

	MeshComponent->SetOverlayMaterial(TargetOverlayMaterial);
}

void ATargetActor_GroundPick::RestoreTargetOverlay(int32 OverlayEntryIndex)
{
	if (!OverlayEntries.IsValidIndex(OverlayEntryIndex))
	{
		return;
	}

	UMeshComponent* MeshComponent = OverlayEntries[OverlayEntryIndex].MeshComponent.Get();
	if (MeshComponent && MeshComponent->GetOverlayMaterial() == OverlayEntries[OverlayEntryIndex].AppliedOverlayMaterial)
	{
		MeshComponent->SetOverlayMaterial(OverlayEntries[OverlayEntryIndex].PreviousOverlayMaterial);
	}

	OverlayEntries.RemoveAtSwap(OverlayEntryIndex);
}

bool ATargetActor_GroundPick::HasOverlayEntryFor(UMeshComponent* MeshComponent) const
{
	for (const FGroundPickOverlayEntry& OverlayEntry : OverlayEntries)
	{
		if (OverlayEntry.MeshComponent.Get() == MeshComponent)
		{
			return true;
		}
	}

	return false;
}

bool ATargetActor_GroundPick::IsValidTargetActor(AActor* TargetActor, const IGenericTeamAgentInterface* OwnerTeamInterface) const
{
	if (!TargetActor)
	{
		return false;
	}

	if (OwnerTeamInterface)
	{
		const ETeamAttitude::Type TeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*TargetActor);
		if (TeamAttitude == ETeamAttitude::Friendly && !bShouldTargetFriendly)
		{
			return false;
		}

		if (TeamAttitude == ETeamAttitude::Hostile && !bShouldTargetEnemy)
		{
			return false;
		}
	}

	return true;
}

UMeshComponent* ATargetActor_GroundPick::GetOverlayMeshComponent(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return nullptr;
	}

	if (const ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		return TargetCharacter->GetMesh();
	}

	return TargetActor->FindComponentByClass<UMeshComponent>();
}
