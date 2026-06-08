// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_CountessExecute.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Components/MeshComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "Push/Player/Characters/PushPlayerCharacter.h"

UGA_CountessExecute::UGA_CountessExecute()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::Ability_Countess_Execute);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
}

void UGA_CountessExecute::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	ObservedAbilitySpecHandle = Spec.Handle;
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		BindAbilitySpecDirtiedDelegate(Cast<UPushAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()));
	}

	RefreshPassiveOverlayScanForSpec(Spec);
}

void UGA_CountessExecute::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	ClearAbilitySpecDirtiedDelegate();
	StopLocalOverlayScan();
	ClearExecutableOverlays();
	ObservedAbilitySpecHandle = FGameplayAbilitySpecHandle();

	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UGA_CountessExecute::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	bExecuting = false;
	ExecutingTarget.Reset();
	SetCanBeCanceled(false);
	SetShouldBlockOtherAbilities(false);

	AActor* TargetActor = nullptr;
	if (!TryFindBestExecuteTarget(TargetActor))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	TryExecuteTarget(TargetActor);

	if (!bExecuting)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_CountessExecute::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	RestoreExecuteMovement();
	ExecutingTarget.Reset();
	bExecuting = false;
	SetCanBeCanceled(false);
	SetShouldBlockOtherAbilities(false);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_CountessExecute::HandleExecuteMontageEnded()
{
	FinishExecute();
}

void UGA_CountessExecute::HandleExecuteMontageCancelled()
{
	FinishExecute();
}

void UGA_CountessExecute::HandleExecuteDamageEvent(FGameplayEventData EventData)
{
	if (!K2_HasAuthority() || !bExecuting)
	{
		return;
	}

	AActor* ExpectedTarget = ExecutingTarget.Get();
	if (!ExpectedTarget || !DamageEffectClass)
	{
		if (!DamageEffectClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("CountessExecute: DamageEffectClass is not set."));
		}
		return;
	}

	const int32 HitResultCount = UAbilitySystemBlueprintLibrary::GetDataCountFromTargetData(EventData.TargetData);
	for (int32 Index = 0; Index < HitResultCount; ++Index)
	{
		const FHitResult HitResult = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(EventData.TargetData, Index);
		AActor* HitActor = HitResult.GetActor();
		if (HitActor != ExpectedTarget || !IsValidExecuteTarget(HitActor))
		{
			continue;
		}

		ApplyGameplayEffectToHitResultActor(
			HitResult,
			DamageEffectClass,
			GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		return;
	}
}

void UGA_CountessExecute::SetupExecuteDamageWait()
{
	if (!K2_HasAuthority())
	{
		return;
	}

	UAbilityTask_WaitGameplayEvent* WaitDamageEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this,
			PushGameplayTags::GameplayEvent_Ability_Countess_Execute_Damage);

	WaitDamageEventTask->EventReceived.AddDynamic(this, &ThisClass::HandleExecuteDamageEvent);
	WaitDamageEventTask->ReadyForActivation();
}

void UGA_CountessExecute::BindAbilitySpecDirtiedDelegate(UPushAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent || ObservedAbilitySystemComponent.Get() == AbilitySystemComponent)
	{
		return;
	}

	ClearAbilitySpecDirtiedDelegate();
	ObservedAbilitySystemComponent = AbilitySystemComponent;
	AbilitySpecDirtiedDelegateHandle =
		AbilitySystemComponent->AbilitySpecDirtiedCallbacks.AddUObject(this, &ThisClass::HandleAbilitySpecDirtied);
}

void UGA_CountessExecute::ClearAbilitySpecDirtiedDelegate()
{
	if (UPushAbilitySystemComponent* AbilitySystemComponent = ObservedAbilitySystemComponent.Get())
	{
		if (AbilitySpecDirtiedDelegateHandle.IsValid())
		{
			AbilitySystemComponent->AbilitySpecDirtiedCallbacks.Remove(AbilitySpecDirtiedDelegateHandle);
		}
	}

	AbilitySpecDirtiedDelegateHandle.Reset();
	ObservedAbilitySystemComponent.Reset();
}

void UGA_CountessExecute::HandleAbilitySpecDirtied(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Handle == ObservedAbilitySpecHandle)
	{
		RefreshPassiveOverlayScanForSpec(AbilitySpec);
	}
}

void UGA_CountessExecute::RefreshPassiveOverlayScanForSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Level > 0)
	{
		StartLocalOverlayScan();
	}
	else
	{
		StopLocalOverlayScan();
		ClearExecutableOverlays();
	}
}

void UGA_CountessExecute::StartLocalOverlayScan()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || AvatarActor->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (!AvatarPawn || !AvatarPawn->IsLocallyControlled())
	{
		return;
	}

	if (!ExecutableOverlayMaterial)
	{
		ClearExecutableOverlays();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RefreshExecutableOverlays();
	World->GetTimerManager().ClearTimer(OverlayScanTimerHandle);
	World->GetTimerManager().SetTimer(
		OverlayScanTimerHandle,
		this,
		&ThisClass::RefreshExecutableOverlays,
		FMath::Max(ScanInterval, 0.05f),
		true);
}

void UGA_CountessExecute::StopLocalOverlayScan()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(OverlayScanTimerHandle);
	}
}

void UGA_CountessExecute::RefreshExecutableOverlays()
{
	if (!ExecutableOverlayMaterial || IsOwnerUnableToExecute())
	{
		ClearExecutableOverlays();
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!AvatarActor || !World)
	{
		ClearExecutableOverlays();
		return;
	}

	RestoreInvalidOverlayEntries();

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CountessExecuteOverlayScan), false, AvatarActor);

	TargetScanOverlapResults.Reset();
	World->OverlapMultiByObjectType(
		TargetScanOverlapResults,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TargetScanRange),
		QueryParams);

	DesiredOverlayMeshes.Reset();
	DesiredOverlayMeshes.Reserve(TargetScanOverlapResults.Num());
	for (const FOverlapResult& OverlapResult : TargetScanOverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!IsValidExecuteTarget(TargetActor))
		{
			continue;
		}

		if (UMeshComponent* MeshComponent = GetOverlayMeshComponent(TargetActor))
		{
			DesiredOverlayMeshes.Add(MeshComponent);
			ApplyExecutableOverlay(MeshComponent);
		}
	}

	for (int32 Index = OverlayEntries.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponent = OverlayEntries[Index].MeshComponent.Get();
		if (!MeshComponent || !DesiredOverlayMeshes.Contains(MeshComponent))
		{
			RestoreExecutableOverlay(Index);
		}
	}

	DesiredOverlayMeshes.Reset();
}

void UGA_CountessExecute::ClearExecutableOverlays()
{
	for (int32 Index = OverlayEntries.Num() - 1; Index >= 0; --Index)
	{
		RestoreExecutableOverlay(Index);
	}
}

void UGA_CountessExecute::ApplyExecutableOverlay(UMeshComponent* MeshComponent)
{
	if (!MeshComponent || HasOverlayEntryFor(MeshComponent))
	{
		return;
	}

	FCountessExecuteOverlayEntry OverlayEntry;
	OverlayEntry.MeshComponent = MeshComponent;
	OverlayEntry.PreviousOverlayMaterial = MeshComponent->GetOverlayMaterial();
	OverlayEntries.Add(OverlayEntry);

	MeshComponent->SetOverlayMaterial(ExecutableOverlayMaterial);
}

void UGA_CountessExecute::RestoreExecutableOverlay(int32 OverlayEntryIndex)
{
	if (!OverlayEntries.IsValidIndex(OverlayEntryIndex))
	{
		return;
	}

	UMeshComponent* MeshComponent = OverlayEntries[OverlayEntryIndex].MeshComponent.Get();
	if (MeshComponent && MeshComponent->GetOverlayMaterial() == ExecutableOverlayMaterial)
	{
		MeshComponent->SetOverlayMaterial(OverlayEntries[OverlayEntryIndex].PreviousOverlayMaterial);
	}

	OverlayEntries.RemoveAtSwap(OverlayEntryIndex);
}

void UGA_CountessExecute::RestoreInvalidOverlayEntries()
{
	for (int32 Index = OverlayEntries.Num() - 1; Index >= 0; --Index)
	{
		UMeshComponent* MeshComponent = OverlayEntries[Index].MeshComponent.Get();
		if (!MeshComponent || !MeshComponent->GetOwner() || !IsValidExecuteTarget(MeshComponent->GetOwner()))
		{
			RestoreExecutableOverlay(Index);
		}
	}
}

bool UGA_CountessExecute::HasOverlayEntryFor(UMeshComponent* MeshComponent) const
{
	for (const FCountessExecuteOverlayEntry& OverlayEntry : OverlayEntries)
	{
		if (OverlayEntry.MeshComponent.Get() == MeshComponent)
		{
			return true;
		}
	}

	return false;
}

bool UGA_CountessExecute::TryFindBestExecuteTarget(AActor*& OutTargetActor) const
{
	OutTargetActor = nullptr;

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	if (!AvatarActor || !World || IsOwnerUnableToExecute())
	{
		return false;
	}

	FVector ViewLocation;
	FVector ViewDirection;
	if (!TryGetAimView(ViewLocation, ViewDirection))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CountessExecuteTargetScan), false, AvatarActor);

	TargetScanOverlapResults.Reset();
	World->OverlapMultiByObjectType(
		TargetScanOverlapResults,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TargetScanRange),
		QueryParams);

	const float MinAimDot = FMath::Cos(FMath::DegreesToRadians(AimHalfAngleDegrees));
	float BestAimDot = MinAimDot;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& OverlapResult : TargetScanOverlapResults)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (!IsValidExecuteTarget(TargetActor))
		{
			continue;
		}

		const FVector TargetLocation = GetTargetAnchorLocation(TargetActor);
		const FVector DirectionToTarget = (TargetLocation - ViewLocation).GetSafeNormal();
		const float AimDot = FVector::DotProduct(ViewDirection, DirectionToTarget);
		if (AimDot < MinAimDot)
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(AvatarActor->GetActorLocation(), TargetLocation);
		if (!OutTargetActor || AimDot > BestAimDot || (FMath::IsNearlyEqual(AimDot, BestAimDot) && DistanceSq < BestDistanceSq))
		{
			OutTargetActor = TargetActor;
			BestAimDot = AimDot;
			BestDistanceSq = DistanceSq;
		}
	}

	return OutTargetActor != nullptr;
}

bool UGA_CountessExecute::IsValidExecuteTarget(AActor* TargetActor) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!TargetActor || TargetActor == AvatarActor || !AvatarActor)
	{
		return false;
	}

	// Execute is intentionally hero-only; minions use their own combat/death flow.
	if (!TargetActor->IsA<APushPlayerCharacter>())
	{
		return false;
	}

	if (FVector::DistSquared(AvatarActor->GetActorLocation(), TargetActor->GetActorLocation()) > FMath::Square(TargetScanRange))
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC || TargetASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead))
	{
		return false;
	}

	if (const IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(AvatarActor))
	{
		if (OwnerTeamInterface->GetTeamAttitudeTowards(*TargetActor) != ETeamAttitude::Hostile)
		{
			return false;
		}
	}

	const float MaxHealth = TargetASC->GetNumericAttribute(UPushAttributeSet::GetMaxHealthAttribute());
	const float Health = TargetASC->GetNumericAttribute(UPushAttributeSet::GetHealthAttribute());
	if (MaxHealth <= 0.f || Health <= 0.f)
	{
		return false;
	}

	return Health / MaxHealth <= ExecuteHealthRatio;
}

bool UGA_CountessExecute::IsOwnerUnableToExecute() const
{
	const UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	return OwnerASC
		&& (OwnerASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead)
			|| OwnerASC->HasMatchingGameplayTag(PushGameplayTags::Status_Stun));
}

bool UGA_CountessExecute::TryGetAimView(FVector& OutViewLocation, FVector& OutViewDirection) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	if (const APawn* AvatarPawn = Cast<APawn>(AvatarActor))
	{
		if (AController* Controller = AvatarPawn->GetController())
		{
			FRotator ViewRotation;
			Controller->GetPlayerViewPoint(OutViewLocation, ViewRotation);
			OutViewDirection = ViewRotation.Vector().GetSafeNormal();
			return !OutViewDirection.IsNearlyZero();
		}
	}

	OutViewLocation = AvatarActor->GetActorLocation();
	OutViewDirection = AvatarActor->GetActorForwardVector().GetSafeNormal();
	return !OutViewDirection.IsNearlyZero();
}

UMeshComponent* UGA_CountessExecute::GetOverlayMeshComponent(AActor* TargetActor) const
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

FVector UGA_CountessExecute::GetTargetAnchorLocation(AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return FVector::ZeroVector;
	}

	if (UMeshComponent* MeshComponent = GetOverlayMeshComponent(TargetActor))
	{
		if (TargetAnchorSocketName != NAME_None && MeshComponent->DoesSocketExist(TargetAnchorSocketName))
		{
			return MeshComponent->GetSocketLocation(TargetAnchorSocketName);
		}
	}

	return TargetActor->GetActorLocation();
}

bool UGA_CountessExecute::TryTeleportBehindTarget(AActor* TargetActor) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !TargetActor)
	{
		return false;
	}

	const FVector TargetAnchorLocation = GetTargetAnchorLocation(TargetActor);
	const FVector TargetForward = TargetActor->GetActorForwardVector().GetSafeNormal();
	const FVector BehindDirection = TargetForward.IsNearlyZero()
		? (AvatarActor->GetActorLocation() - TargetAnchorLocation).GetSafeNormal()
		: -TargetForward;

	if (BehindDirection.IsNearlyZero())
	{
		return false;
	}

	const FVector BaseTeleportLocation =
		TargetAnchorLocation + BehindDirection * BehindDistance + FVector::UpVector * TeleportVerticalOffset;

	FRotator TeleportRotation = (TargetAnchorLocation - BaseTeleportLocation).Rotation();
	TeleportRotation.Pitch = 0.f;
	TeleportRotation.Roll = 0.f;

	if (ACharacter* OwningCharacter = Cast<ACharacter>(AvatarActor))
	{
		if (UCharacterMovementComponent* MovementComponent = OwningCharacter->GetCharacterMovement())
		{
			MovementComponent->StopMovementImmediately();
		}
	}

	const FVector RightDirection = FVector::CrossProduct(FVector::UpVector, BehindDirection).GetSafeNormal();
	const FVector TeleportDirections[] =
	{
		BehindDirection,
		RightDirection,
		-RightDirection,
		-BehindDirection
	};

	for (const FVector& TeleportDirection : TeleportDirections)
	{
		if (TeleportDirection.IsNearlyZero())
		{
			continue;
		}

		const FVector TeleportLocation =
			TargetAnchorLocation + TeleportDirection * BehindDistance + FVector::UpVector * TeleportVerticalOffset;
		const FRotator CandidateRotation = (TargetAnchorLocation - TeleportLocation).Rotation();
		if (AvatarActor->TeleportTo(TeleportLocation, FRotator(0.f, CandidateRotation.Yaw, 0.f), false, false))
		{
			return true;
		}
	}

	return AvatarActor->TeleportTo(BaseTeleportLocation, TeleportRotation, false, true);
}

void UGA_CountessExecute::TryExecuteTarget(AActor* TargetActor)
{
	if (!ArrivalMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("CountessExecute: ArrivalMontage is not set."));
		return;
	}

	if (!IsValidExecuteTarget(TargetActor))
	{
		return;
	}

	if (!K2_CheckAbilityCost() || !K2_CheckAbilityCooldown())
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	const FTransform AvatarTransform = AvatarActor ? AvatarActor->GetActorTransform() : FTransform::Identity;

	if (!TryTeleportBehindTarget(TargetActor))
	{
		return;
	}

	if (!K2_CommitAbility())
	{
		if (AvatarActor)
		{
			AvatarActor->TeleportTo(AvatarTransform.GetLocation(), AvatarTransform.Rotator(), false, true);
		}
		return;
	}

	ExecutingTarget = TargetActor;
	bExecuting = true;
	SetCanBeCanceled(true);
	SetShouldBlockOtherAbilities(true);
	LockExecuteMovement(TargetActor);
	SetupExecuteDamageWait();
	StartExecuteMontage();
}

void UGA_CountessExecute::StartExecuteMontage()
{
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ArrivalMontage);

	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::HandleExecuteMontageEnded);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::HandleExecuteMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::HandleExecuteMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::HandleExecuteMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UGA_CountessExecute::FinishExecute()
{
	if (!bExecuting)
	{
		return;
	}

	K2_EndAbility();
	RefreshExecutableOverlays();
}

void UGA_CountessExecute::LockExecuteMovement(AActor* TargetActor)
{
	RestoreExecuteMovement();

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
		{
			MovementLockedAvatar = OwnerCharacter;
			PreviousAvatarMovementMode = MovementComponent->MovementMode;
			PreviousAvatarCustomMovementMode = MovementComponent->CustomMovementMode;
			bAvatarMovementLocked = true;
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}

	if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement())
		{
			MovementLockedTarget = TargetCharacter;
			PreviousTargetMovementMode = MovementComponent->MovementMode;
			PreviousTargetCustomMovementMode = MovementComponent->CustomMovementMode;
			bTargetMovementLocked = true;
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
	}
}

void UGA_CountessExecute::RestoreExecuteMovement()
{
	if (bAvatarMovementLocked)
	{
		if (ACharacter* OwnerCharacter = MovementLockedAvatar.Get())
		{
			if (UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement())
			{
				MovementComponent->SetMovementMode(PreviousAvatarMovementMode, PreviousAvatarCustomMovementMode);
			}
		}
	}

	if (bTargetMovementLocked)
	{
		if (ACharacter* TargetCharacter = MovementLockedTarget.Get())
		{
			if (UCharacterMovementComponent* MovementComponent = TargetCharacter->GetCharacterMovement())
			{
				MovementComponent->SetMovementMode(PreviousTargetMovementMode, PreviousTargetCustomMovementMode);
			}
		}
	}

	MovementLockedAvatar.Reset();
	MovementLockedTarget.Reset();
	PreviousAvatarMovementMode = MOVE_None;
	PreviousTargetMovementMode = MOVE_None;
	PreviousAvatarCustomMovementMode = 0;
	PreviousTargetCustomMovementMode = 0;
	bAvatarMovementLocked = false;
	bTargetMovementLocked = false;
}
