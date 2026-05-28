// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Siphon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/Player/Characters/PushPlayerCharacter.h"

UGA_Siphon::UGA_Siphon()
{
	AbilityTags.AddTag(PushGameplayTags::Ability_Countess_Siphon);
	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
}

void UGA_Siphon::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bEndingAbility = false;
	bCommitCooldownOnEnd = false;

	if (!HasValidSiphonConfig())
	{
		K2_EndAbility();
		return;
	}

	if (APushPlayerCharacter* PushPlayerCharacter = Cast<APushPlayerCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (PushPlayerCharacter->GetCharacterMovement()->IsFalling())
		{
			K2_EndAbility();
			return;
		}
	}

	if (!K2_CommitAbilityCost(false))
	{
		K2_EndAbility();
		return;
	}
	bCommitCooldownOnEnd = true;

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		DisableAvatarMovement();
		StartMontageLoop();

		UAbilityTask_WaitInputRelease* WaitInputReleaseTask =
			UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);

		WaitInputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::HandleInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	StartSiphonTimers();
}

void UGA_Siphon::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	bEndingAbility = true;
	StopSiphonTimers();
	RestoreAvatarMovement();

	if (bCommitCooldownOnEnd)
	{
		bCommitCooldownOnEnd = false;
		K2_CommitAbilityCooldown(false, true);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Siphon::HandleInputReleased(float TimeHeld)
{
	if (bEndingAbility)
	{
		return;
	}

	K2_EndAbility();
}

void UGA_Siphon::HandleMontageCompleted()
{
	if (bEndingAbility)
	{
		return;
	}

	StartMontageLoop();
}

void UGA_Siphon::HandleMontageCancelled()
{
	if (bEndingAbility)
	{
		return;
	}

	K2_EndAbility();
}

void UGA_Siphon::StartMontageLoop()
{
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			AbilityMontage
		);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_Siphon::StartSiphonTimers()
{
	if (!K2_HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		K2_EndAbility();
		return;
	}

	PerformDamageTick();

	World->GetTimerManager().SetTimer(
		DamageTimerHandle,
		this,
		&ThisClass::PerformDamageTick,
		DamageInterval,
		true);

	World->GetTimerManager().SetTimer(
		CostTimerHandle,
		this,
		&ThisClass::PerformCostTick,
		CostInterval,
		true);
}

void UGA_Siphon::StopSiphonTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTimerHandle);
		World->GetTimerManager().ClearTimer(CostTimerHandle);
	}
}

void UGA_Siphon::DisableAvatarMovement()
{
	if (bMovementDisabledByAbility)
	{
		return;
	}

	ACharacter* OwnerCharacter = GetOwningAvatarCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	PreviousMovementMode = static_cast<uint8>(MovementComponent->MovementMode);
	PreviousCustomMovementMode = MovementComponent->CustomMovementMode;
	bMovementDisabledByAbility = true;

	MovementComponent->DisableMovement();
}

void UGA_Siphon::RestoreAvatarMovement()
{
	if (!bMovementDisabledByAbility)
	{
		return;
	}

	bMovementDisabledByAbility = false;

	ACharacter* OwnerCharacter = GetOwningAvatarCharacter();
	if (!OwnerCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (ASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead)
			|| ASC->HasMatchingGameplayTag(PushGameplayTags::Status_Stun))
		{
			return;
		}
	}

	const EMovementMode MovementModeToRestore =
		PreviousMovementMode == static_cast<uint8>(MOVE_None)
			? MOVE_Walking
			: static_cast<EMovementMode>(PreviousMovementMode);

	MovementComponent->SetMovementMode(MovementModeToRestore, PreviousCustomMovementMode);
}

void UGA_Siphon::PerformDamageTick()
{
	if (!K2_HasAuthority() || bEndingAbility)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!AvatarActor || !World)
	{
		K2_EndAbility();
		return;
	}

	const FVector Origin = AvatarActor->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SiphonDamageOverlap), false, AvatarActor);

	World->OverlapMultiByObjectType(
		OverlapResults,
		Origin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(DamageRadius),
		QueryParams);

	if (ShouldDrawDebug())
	{
		DrawDebugSphere(World, Origin, DamageRadius, 24, FColor::Purple, false, DamageInterval);
	}

	float TotalHealAmount = 0.f;
	TSet<AActor*> DamagedActors;

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (DamagedActors.Contains(TargetActor) || !IsValidSiphonTarget(TargetActor))
		{
			continue;
		}

		DamagedActors.Add(TargetActor);
		TotalHealAmount += DamageTargetAndGetHealAmount(TargetActor);
	}

	HealOwner(TotalHealAmount);
}

void UGA_Siphon::PerformCostTick()
{
	if (!K2_HasAuthority() || bEndingAbility)
	{
		return;
	}

	if (!K2_CommitAbilityCost(false))
	{
		K2_EndAbility();
	}
}

bool UGA_Siphon::HasValidSiphonConfig() const
{
	bool bIsValid = true;

	if (!AbilityMontage)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: AbilityMontage is not set."));
		bIsValid = false;
	}

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: DamageEffectClass is not set."));
		bIsValid = false;
	}

	if (!GetCostGameplayEffect())
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: Cost GameplayEffect is not set."));
		bIsValid = false;
	}

	if (DamageRadius <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: DamageRadius must be greater than zero."));
		bIsValid = false;
	}

	if (DamageInterval <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: DamageInterval must be greater than zero."));
		bIsValid = false;
	}

	if (CostInterval <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: CostInterval must be greater than zero."));
		bIsValid = false;
	}

	if (HealRatio < 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("Siphon: HealRatio must be zero or greater."));
		bIsValid = false;
	}

	return bIsValid;
}

bool UGA_Siphon::IsValidSiphonTarget(AActor* TargetActor) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!TargetActor || TargetActor == AvatarActor)
	{
		return false;
	}

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		return false;
	}

	if (const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(AvatarActor))
	{
		return OwnerTeamInterface->GetTeamAttitudeTowards(*TargetActor) == ETeamAttitude::Hostile;
	}

	return true;
}

float UGA_Siphon::DamageTargetAndGetHealAmount(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return 0.f;
	}

	const float HealthBefore = TargetASC->GetNumericAttribute(UPushAttributeSet::GetHealthAttribute());
	if (HealthBefore <= 0.f)
	{
		return 0.f;
	}

	UPrimitiveComponent* TargetPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	const FVector AvatarLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector HitNormal = (TargetLocation - AvatarLocation).GetSafeNormal();
	const FHitResult HitResult(TargetActor, TargetPrimitive, TargetLocation, HitNormal);

	ApplyGameplayEffectToHitResultActor(
		HitResult,
		DamageEffectClass,
		GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo)
	);

	const float HealthAfter = TargetASC->GetNumericAttribute(UPushAttributeSet::GetHealthAttribute());
	const float DamageDealt = FMath::Max(HealthBefore - HealthAfter, 0.f);

	return DamageDealt * HealRatio;
}

void UGA_Siphon::HealOwner(float HealAmount) const
{
	if (HealAmount <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC)
	{
		return;
	}

	const float CurrentHealth = OwnerASC->GetNumericAttribute(UPushAttributeSet::GetHealthAttribute());
	const float MaxHealth = OwnerASC->GetNumericAttribute(UPushAttributeSet::GetMaxHealthAttribute());
	const float ClampedHealAmount = FMath::Min(HealAmount, MaxHealth - CurrentHealth);

	if (ClampedHealAmount > 0.f)
	{
		OwnerASC->ApplyModToAttribute(
			UPushAttributeSet::GetHealthAttribute(),
			EGameplayModOp::Additive,
			ClampedHealAmount);
	}
}
