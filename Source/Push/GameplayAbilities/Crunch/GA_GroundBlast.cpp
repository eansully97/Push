// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_GroundBlast.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Engine/OverlapResult.h"
#include "GenericTeamAgentInterface.h"
#include "Push/GAS/Targeting/TargetActor_GroundPick.h"
#include "Push/PushGameplayTags.h"

UGA_GroundBlast::UGA_GroundBlast()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::Ability_Crunch_GroundBlast);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(PushGameplayTags::Status_Aiming);
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
}

void UGA_GroundBlast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                      const FGameplayEventData* TriggerEventData)
{
	if (!HasValidGroundBlastConfig())
	{
		K2_EndAbility();
		return;
	}

	if (!HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
	{
		K2_EndAbility();
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageWaitTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, TargetingAbilityMontage);
	MontageWaitTask->OnBlendOut.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnCancelled.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnInterrupted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->OnCompleted.AddDynamic(this, &ThisClass::K2_EndAbility);
	MontageWaitTask->ReadyForActivation();

	UAbilityTask_WaitTargetData* WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &ThisClass::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &ThisClass::TargetCancelled);

	AGameplayAbilityTargetActor* TargetActor = nullptr;
	const bool bSpawnedTargetActor = WaitTargetDataTask->BeginSpawningActor(this, TargetActorClass, TargetActor);
	if (bSpawnedTargetActor && TargetActor)
	{
		if (ATargetActor_GroundPick* GroundPickActor = Cast<ATargetActor_GroundPick>(TargetActor))
		{
			GroundPickActor->SetShouldDrawDebug(ShouldDrawDebug());
			GroundPickActor->SetTargetAreaRadius(TargetAreaRadius);
			GroundPickActor->SetTargetTraceRange(TargetTraceRange);
		}
	
		WaitTargetDataTask->FinishSpawningActor(this, TargetActor);
	}
	else if (CurrentActorInfo && CurrentActorInfo->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: failed to spawn target actor %s."),
			TargetActorClass ? *TargetActorClass->GetName() : TEXT("None"));
		K2_EndAbility();
		return;
	}

	WaitTargetDataTask->ReadyForActivation();
}

void UGA_GroundBlast::TargetConfirmed(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	FVector TargetLocation;
	if (!TryGetValidatedTargetLocation(TargetDataHandle, TargetLocation))
	{
		K2_EndAbility();
		return;
	}

	if (!K2_CommitAbilityCost())
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		const FGameplayAbilityTargetDataHandle ServerTargetData = BuildServerTargetData(TargetLocation);
		BP_ApplyGameplayEffectToTarget(ServerTargetData, DamageEffectDef.DamageEffectClass, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		PushTargets(ServerTargetData, DamageEffectDef.PushVelocity);

		FGameplayCueParameters CueParams;
		CueParams.Location = TargetLocation;
		CueParams.RawMagnitude = TargetAreaRadius;

		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
			ASC->ExecuteGameplayCue(PushGameplayTags::GameplayCue_CameraShake);
		}

		K2_CommitAbilityCooldown();
	}

	if (CastAbilityMontage)
	{
		if (UAnimInstance* OwnerAnimInstance = GetOwnerAnimInstance())
		{
			OwnerAnimInstance->Montage_Play(CastAbilityMontage);
		}
	}

	K2_EndAbility();
}

void UGA_GroundBlast::TargetCancelled(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	K2_EndAbility();
}

bool UGA_GroundBlast::HasValidGroundBlastConfig() const
{
	bool bIsValid = true;

	if (!TargetingAbilityMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: TargetingAbilityMontage is not set."));
		bIsValid = false;
	}

	if (!TargetActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: TargetActorClass is not set."));
		bIsValid = false;
	}

	if (!DamageEffectDef.DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: DamageEffectClass is not set."));
		bIsValid = false;
	}

	if (TargetAreaRadius <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: TargetAreaRadius must be greater than zero."));
		bIsValid = false;
	}

	if (TargetTraceRange <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: TargetTraceRange must be greater than zero."));
		bIsValid = false;
	}

	if (!GameplayCueTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundBlast: GameplayCueTag is not set."));
		bIsValid = false;
	}

	return bIsValid;
}

bool UGA_GroundBlast::TryGetValidatedTargetLocation(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FVector& OutTargetLocation) const
{
	if (TargetDataHandle.Num() < 1)
		return false;

	const int32 LocationDataIndex = TargetDataHandle.Num() > 1 ? 1 : 0;
	const FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, LocationDataIndex);
	OutTargetLocation = HitResult.ImpactPoint;
	if (OutTargetLocation.IsNearlyZero())
	{
		OutTargetLocation = HitResult.Location;
	}

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
		return false;

	const float MaxDistance = TargetTraceRange + TargetAreaRadius;
	return !OutTargetLocation.IsNearlyZero()
		&& FVector::DistSquared(AvatarActor->GetActorLocation(), OutTargetLocation) <= FMath::Square(MaxDistance);
}

FGameplayAbilityTargetDataHandle UGA_GroundBlast::BuildServerTargetData(const FVector& TargetLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return {};
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GroundBlastTargetOverlap), false, AvatarActor);
	World->OverlapMultiByObjectType(
		OverlapResults,
		TargetLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TargetAreaRadius),
		QueryParams);

	TSet<AActor*> TargetActors;
	const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(AvatarActor);
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!TargetActor || TargetActor == AvatarActor)
		{
			continue;
		}

		if (OwnerTeamInterface && OwnerTeamInterface->GetTeamAttitudeTowards(*TargetActor) != ETeamAttitude::Hostile)
		{
			continue;
		}

		TargetActors.Add(TargetActor);
	}

	FGameplayAbilityTargetDataHandle TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActorArray(TargetActors.Array(), false);
	FGameplayAbilityTargetData_SingleTargetHit* HitLocation = new FGameplayAbilityTargetData_SingleTargetHit;
	HitLocation->HitResult.ImpactPoint = TargetLocation;
	HitLocation->HitResult.Location = TargetLocation;
	TargetData.Add(HitLocation);
	return TargetData;
}
