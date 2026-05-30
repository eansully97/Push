// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_CountessExecute.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
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

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle);
	}
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

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		SetupInputWait();
	}

	StartLocalOverlayScan();
}

void UGA_CountessExecute::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	RestoreExecuteMovement();
	StopLocalOverlayScan();
	ClearExecutableOverlays();
	ExecutingTarget.Reset();
	bExecuting = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_CountessExecute::HandleInputPressed(float TimeWaited)
{
	if (bExecuting)
	{
		return;
	}

	AActor* TargetActor = nullptr;
	if (!TryFindBestExecuteTarget(TargetActor))
	{
		SetupInputWait();
		return;
	}

	TryExecuteTarget(TargetActor);

	if (!bExecuting)
	{
		SetupInputWait();
	}
}

void UGA_CountessExecute::HandleExecuteMontageEnded()
{
	FinishExecute();
}

void UGA_CountessExecute::HandleExecuteMontageCancelled()
{
	FinishExecute();
}

void UGA_CountessExecute::SetupInputWait()
{
	if (bExecuting)
	{
		return;
	}

	UAbilityTask_WaitInputPress* WaitInputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
	WaitInputPressTask->OnPress.AddDynamic(this, &ThisClass::HandleInputPressed);
	WaitInputPressTask->ReadyForActivation();
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
	World->GetTimerManager().SetTimer(
		OverlayScanTimerHandle,
		this,
		&ThisClass::RefreshExecutableOverlays,
		ScanInterval,
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

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CountessExecuteOverlayScan), false, AvatarActor);

	World->OverlapMultiByObjectType(
		OverlapResults,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TargetScanRange),
		QueryParams);

	TSet<UMeshComponent*> DesiredOverlayMeshes;
	for (const FOverlapResult& OverlapResult : OverlapResults)
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

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CountessExecuteTargetScan), false, AvatarActor);

	World->OverlapMultiByObjectType(
		OverlapResults,
		AvatarActor->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TargetScanRange),
		QueryParams);

	const float MinAimDot = FMath::Cos(FMath::DegreesToRadians(AimHalfAngleDegrees));
	float BestAimDot = MinAimDot;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (const FOverlapResult& OverlapResult : OverlapResults)
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
	TArray<FVector> SideDirections{RightDirection, -RightDirection, -BehindDirection};
	const int32 SideDirectionIndex = FMath::RandRange(0, SideDirections.Num() - 1);
	SideDirections.Swap(0, SideDirectionIndex);

	TArray<FVector> TeleportDirections{BehindDirection};
	TeleportDirections.Append(SideDirections);

	for (const FVector& TeleportDirection : TeleportDirections)
	{
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

FHitResult UGA_CountessExecute::BuildTargetHitResult(AActor* TargetActor) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UPrimitiveComponent* TargetPrimitive = TargetActor ? Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()) : nullptr;
	const FVector TargetLocation = TargetActor ? GetTargetAnchorLocation(TargetActor) : FVector::ZeroVector;
	const FVector HitNormal = AvatarActor && TargetActor
		? (TargetActor->GetActorLocation() - AvatarActor->GetActorLocation()).GetSafeNormal()
		: FVector::ForwardVector;

	return FHitResult(TargetActor, TargetPrimitive, TargetLocation, HitNormal);
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
	ApplyExecuteDamage(TargetActor);
	LockExecuteMovement(TargetActor);
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

	bExecuting = false;
	ExecutingTarget.Reset();
	SetCanBeCanceled(false);
	SetShouldBlockOtherAbilities(false);
	RestoreExecuteMovement();
	RefreshExecutableOverlays();
	SetupInputWait();
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

void UGA_CountessExecute::ApplyExecuteDamage(AActor* TargetActor)
{
	if (!K2_HasAuthority())
	{
		return;
	}

	if (!IsValidExecuteTarget(TargetActor))
	{
		return;
	}

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CountessExecute: DamageEffectClass is not set."));
		return;
	}

	ApplyGameplayEffectToHitResultActor(
		BuildTargetHitResult(TargetActor),
		DamageEffectClass,
		GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
}
