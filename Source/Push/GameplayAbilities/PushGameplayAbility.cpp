// Fill out your copyright notice in the Description page of Project Settings.


#include "PushGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "StatusAbilities/GA_Status_Launched.h"


ACharacter* UPushGameplayAbility::GetOwningAvatarCharacter()
{
	if (!AvatarCharacter)
	{
		AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	}
	return AvatarCharacter;
}

UAnimInstance* UPushGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerMeshComp = GetOwningComponentFromActorInfo();
	if (OwnerMeshComp)
	{
		return OwnerMeshComp->GetAnimInstance();
	}
	return nullptr;
}

TArray<FHitResult> UPushGameplayAbility::GetHitResultFromSweepLocationTargetData(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle, float SphereSweepRadius, ETeamAttitude::Type TargetTeam,
	bool bDrawDebug, bool bIgnoreSelf) const
{
	TArray<FHitResult> OutHitResults;
	TSet<AActor*> HitActors;

	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());

	for (const auto TargetData : TargetDataHandle.Data)
	{
		FVector StartLoc = TargetData->GetOrigin().GetTranslation();
		FVector EndLoc = TargetData->GetEndPoint();

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

		TArray<AActor*> ActorsToIgnore;
		if (bIgnoreSelf)
		{
			ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
		}

		EDrawDebugTrace::Type DrawDebugTrace = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		TArray<FHitResult> HitResults;
		UKismetSystemLibrary::SphereTraceMultiForObjects(this, StartLoc, EndLoc, SphereSweepRadius, ObjectTypes, false, ActorsToIgnore, DrawDebugTrace, HitResults, false);

		for (const auto HitResult : HitResults)
		{
			if (HitActors.Contains(HitResult.GetActor()))
			{
				continue;
			}

			if (OwnerTeamInterface)
			{
				ETeamAttitude::Type OtherActorTeamAttitude = OwnerTeamInterface->GetTeamAttitudeTowards(*HitResult.GetActor());
				if (OtherActorTeamAttitude != TargetTeam)
				{
					continue;
				}
			}

			HitActors.Add(HitResult.GetActor());
			OutHitResults.Add(HitResult);
		}
	}
	
	return OutHitResults;
}

void UPushGameplayAbility::PushSelf(const FVector& PushVelocity)
{
	if (ACharacter* OwningAvatarCharacter = GetOwningAvatarCharacter())
	{
		OwningAvatarCharacter->LaunchCharacter(PushVelocity, true, true);
	}
}

void UPushGameplayAbility::PushTarget(AActor* Target, const FVector& PushVelocity)
{
	if (!Target) return;

	FGameplayEventData EventData;

	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit;

	FHitResult HitResult;
	HitResult.ImpactNormal = PushVelocity.GetSafeNormal();

	HitData->HitResult = HitResult;

	EventData.TargetData.Add(HitData);
	EventData.EventMagnitude = PushVelocity.Size();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Target,
		UGA_Status_Launched::GetLaunchAbilityActivationTag(),
		EventData
	);
}
