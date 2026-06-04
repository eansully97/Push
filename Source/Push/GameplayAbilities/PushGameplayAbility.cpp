// Fill out your copyright notice in the Description page of Project Settings.


#include "PushGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "Push/GameplayAbilities/StatusAbilities/GA_Status_Launched.h"


UPushGameplayAbility::UPushGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationBlockedTags.AddTag(PushGameplayTags::Status_Stun);
}

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

bool UPushGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (AbilitySpec && AbilitySpec->Level <= 0)
		{
			return false;
		}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
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

void UPushGameplayAbility::ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult,
	TSubclassOf<UGameplayEffect> Effect, int32 Level)
{
	AActor* TargetActor = HitResult.GetActor();
	if (!Effect || !TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s could not apply gameplay effect: Effect=%s Target=%s."),
			*GetName(),
			Effect ? *Effect->GetName() : TEXT("None"),
			TargetActor ? *TargetActor->GetName() : TEXT("None"));
		return;
	}

	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(Effect, Level);
	if (!EffectSpecHandle.IsValid() || !EffectSpecHandle.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to create gameplay effect spec for %s."),
			*GetName(),
			*Effect->GetName());
		return;
	}

	FGameplayEffectContextHandle EffectContextHandle = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContextHandle.AddHitResult(HitResult);

	EffectSpecHandle.Data->SetContext(EffectContextHandle);

	const TArray<FActiveGameplayEffectHandle> AppliedEffectHandles =
		ApplyGameplayEffectSpecToTarget(
			GetCurrentAbilitySpecHandle(),
			CurrentActorInfo,
			CurrentActivationInfo,
			EffectSpecHandle,
			UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(TargetActor));

	if (!AppliedEffectHandles.IsEmpty())
	{
		if (UPushAbilitySystemComponent* PushASC = Cast<UPushAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
		{
			PushASC->AuthBreakStealth();
		}
	}
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

void UPushGameplayAbility::PushTargets(TArray<AActor*>& Targets, const FVector& PushVelocity)
{
	for (AActor* Target : Targets)
	{
		PushTarget(Target, PushVelocity);
	}
}

void UPushGameplayAbility::PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	const FVector& PushVelocity)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	PushTargets(Targets, PushVelocity);
}
