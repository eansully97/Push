// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Push/GameplayAbilities/PushGameplayAbility.h"
#include "GA_CountessExecute.generated.h"

class UMaterialInterface;
class UMeshComponent;
class ACharacter;
class UPushAbilitySystemComponent;

USTRUCT()
struct FCountessExecuteOverlayEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY()
	UMaterialInterface* PreviousOverlayMaterial = nullptr;
};

/**
 * Passive Countess execute helper. Highlights executable enemies locally, then
 * uses the bound input to teleport behind the best aimed target and play a montage.
 */
UCLASS()
class PUSH_API UGA_CountessExecute : public UPushGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CountessExecute();

	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ArrivalMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExecuteHealthRatio = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (ClampMin = "0.0", Units = "cm"))
	float TargetScanRange = 1600.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
	float AimHalfAngleDegrees = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting", meta = (ClampMin = "0.01", Units = "s"))
	float ScanInterval = 0.12f;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	FName TargetAnchorSocketName = TEXT("root");

	UPROPERTY(EditDefaultsOnly, Category = "Teleport", meta = (ClampMin = "0.0", Units = "cm"))
	float BehindDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport", meta = (Units = "cm"))
	float TeleportVerticalOffset = 88.f;

	UPROPERTY(EditDefaultsOnly, Category = "Cosmetics")
	UMaterialInterface* ExecutableOverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UFUNCTION()
	void HandleExecuteMontageEnded();

	UFUNCTION()
	void HandleExecuteMontageCancelled();

	UFUNCTION()
	void HandleExecuteDamageEvent(FGameplayEventData EventData);

	void SetupExecuteDamageWait();
	void BindAbilitySpecDirtiedDelegate(UPushAbilitySystemComponent* AbilitySystemComponent);
	void ClearAbilitySpecDirtiedDelegate();
	void HandleAbilitySpecDirtied(const FGameplayAbilitySpec& AbilitySpec);
	void RefreshPassiveOverlayScanForSpec(const FGameplayAbilitySpec& AbilitySpec);
	void StartLocalOverlayScan();
	void StopLocalOverlayScan();
	void RefreshExecutableOverlays();
	void ClearExecutableOverlays();
	void ApplyExecutableOverlay(UMeshComponent* MeshComponent);
	void RestoreExecutableOverlay(int32 OverlayEntryIndex);
	void RestoreInvalidOverlayEntries();
	bool HasOverlayEntryFor(UMeshComponent* MeshComponent) const;

	bool TryFindBestExecuteTarget(AActor*& OutTargetActor) const;
	bool IsValidExecuteTarget(AActor* TargetActor) const;
	bool IsOwnerUnableToExecute() const;
	bool TryGetAimView(FVector& OutViewLocation, FVector& OutViewDirection) const;
	UMeshComponent* GetOverlayMeshComponent(AActor* TargetActor) const;
	FVector GetTargetAnchorLocation(AActor* TargetActor) const;
	bool TryTeleportBehindTarget(AActor* TargetActor) const;
	void TryExecuteTarget(AActor* TargetActor);
	void StartExecuteMontage();
	void FinishExecute();
	void LockExecuteMovement(AActor* TargetActor);
	void RestoreExecuteMovement();

	FTimerHandle OverlayScanTimerHandle;
	FDelegateHandle AbilitySpecDirtiedDelegateHandle;

	UPROPERTY()
	TArray<FCountessExecuteOverlayEntry> OverlayEntries;

	TWeakObjectPtr<UPushAbilitySystemComponent> ObservedAbilitySystemComponent;
	TWeakObjectPtr<AActor> ExecutingTarget;
	TWeakObjectPtr<ACharacter> MovementLockedAvatar;
	TWeakObjectPtr<ACharacter> MovementLockedTarget;
	FGameplayAbilitySpecHandle ObservedAbilitySpecHandle;
	TEnumAsByte<EMovementMode> PreviousAvatarMovementMode = MOVE_None;
	TEnumAsByte<EMovementMode> PreviousTargetMovementMode = MOVE_None;
	uint8 PreviousAvatarCustomMovementMode = 0;
	uint8 PreviousTargetCustomMovementMode = 0;
	bool bAvatarMovementLocked = false;
	bool bTargetMovementLocked = false;
	bool bExecuting = false;

	mutable TArray<FOverlapResult> TargetScanOverlapResults;
	TSet<UMeshComponent*> DesiredOverlayMeshes;
};
