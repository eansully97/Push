// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_SendTargetGroup.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameplayCueManager.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
	FVector GetFirstNonZeroVector(const FVector& Primary, const FVector& Secondary, const FVector& Fallback)
	{
		if (!Primary.IsNearlyZero())
		{
			return Primary;
		}

		if (!Secondary.IsNearlyZero())
		{
			return Secondary;
		}

		return Fallback;
	}

	FHitResult BuildCueSafeHitResult(const FHitResult& HitResult, AActor* OwnerActor)
	{
		FHitResult SafeHitResult = HitResult;
		AActor* HitActor = SafeHitResult.GetActor();

		const FVector FallbackLocation = HitActor ? HitActor->GetActorLocation() : FVector::ZeroVector;
		const FVector HitLocation = GetFirstNonZeroVector(
			SafeHitResult.ImpactPoint,
			SafeHitResult.Location,
			FallbackLocation);

		SafeHitResult.ImpactPoint = HitLocation;
		SafeHitResult.Location = HitLocation;

		const FVector OwnerToHitDirection =
			OwnerActor && !HitLocation.IsNearlyZero()
				? (HitLocation - OwnerActor->GetActorLocation()).GetSafeNormal()
				: FVector::ZeroVector;

		const FVector HitNormal = GetFirstNonZeroVector(
			SafeHitResult.ImpactNormal,
			SafeHitResult.Normal,
			OwnerToHitDirection.IsNearlyZero() ? FVector::UpVector : OwnerToHitDirection).GetSafeNormal();

		SafeHitResult.ImpactNormal = HitNormal;
		SafeHitResult.Normal = HitNormal;

		return SafeHitResult;
	}
}

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	if (TargetSocketNames.Num() <= 1)
		return;

	FGameplayEventData EventData;
	TSet<AActor*> HitActors;
	AActor* OwnerActor = MeshComp->GetOwner();
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(OwnerActor);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoredActors;
	if (bIgnoreOwner)
	{
		IgnoredActors.Add(OwnerActor);
	}

	const EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	for (int32 i = 1; i < TargetSocketNames.Num(); i++)
	{
		const FVector StartLoc = MeshComp->GetSocketLocation(TargetSocketNames[i - 1]);
		const FVector EndLoc = MeshComp->GetSocketLocation(TargetSocketNames[i]);

		TArray<FHitResult> OutHitResults;
		UKismetSystemLibrary::SphereTraceMultiForObjects(
			MeshComp,
			StartLoc,
			EndLoc,
			SphereRadius,
			ObjectTypes,
			false,
			IgnoredActors,
			DrawDebugTrace,
			OutHitResults,
			false);

		for (const FHitResult& HitResult : OutHitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*HitActor) != TargetTeam)
			{
				continue;
			}

			HitActors.Add(HitActor);

			const FHitResult SafeHitResult = BuildCueSafeHitResult(HitResult, OwnerActor);
			FGameplayAbilityTargetData_SingleTargetHit* TargetHit = new FGameplayAbilityTargetData_SingleTargetHit(SafeHitResult);
			EventData.TargetData.Add(TargetHit);
			SendLocalGameplayCue(SafeHitResult, MeshComp, Animation);
		}
	}

	if (EventTag.IsValid() && OwnerASC)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, EventData);
	}
}

void UAN_SendTargetGroup::SendLocalGameplayCue(const FHitResult& HitResult, USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const
{
	UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;
	CueParams.Instigator = MeshComp->GetOwner();
	CueParams.EffectCauser = MeshComp->GetOwner();
	CueParams.SourceObject = Animation;
	CueParams.PhysicalMaterial = HitResult.PhysMaterial.Get();
	CueParams.TargetAttachComponent = HitResult.GetComponent();
	if (!CueParams.TargetAttachComponent.IsValid())
	{
		CueParams.TargetAttachComponent = HitActor->GetRootComponent();
	}

	FGameplayEffectContextHandle EffectContext(
		UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	EffectContext.AddHitResult(HitResult);
	CueParams.EffectContext = EffectContext;

	for (const FGameplayTag& GameplayCueTag : GameplayCueTriggerTags)
	{
		if (!GameplayCueTag.IsValid())
		{
			continue;
		}

		UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(HitActor, GameplayCueTag, EGameplayCueEvent::Executed, CueParams);
	}
}
