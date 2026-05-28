// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetActor_GroundPick.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/OverlapResult.h"
#include "Push/Push.h"

ATargetActor_GroundPick::ATargetActor_GroundPick()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATargetActor_GroundPick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PrimaryPC && PrimaryPC->IsLocalPlayerController())
	{
		SetActorLocation(GetTargetPoint());
	}
}

void ATargetActor_GroundPick::ConfirmTargetingAndContinue()
{
	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams QueryParams;
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(TargetAreaRadius);
	GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(),FQuat::Identity, QueryParams, CollisionShape);
	TSet<AActor*> TargetActors;

	const IGenericTeamAgentInterface* OwnerTeamInterface = nullptr;
	if (OwningAbility)
	{
		OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwningAbility->GetAvatarActorFromActorInfo());
	}

	for (const auto& OverlapResult : OverlapResults)
	{
		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Friendly && !bShouldTargetFriendly)
			continue;

		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*OverlapResult.GetActor()) == ETeamAttitude::Hostile && !bShouldTargetEnemy)
			continue;
		
		TargetActors.Add(OverlapResult.GetActor());
	}

	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	FGameplayAbilityTargetData_SingleTargetHit* HitLocation = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLocation->HitResult.ImpactPoint = GetActorLocation();
	TargetData.Add(HitLocation);
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
