// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "TargetActor_GroundPick.generated.h"

class UMaterialInterface;
class UMeshComponent;
class UDecalComponent;
class IGenericTeamAgentInterface;

USTRUCT()
struct FGroundPickOverlayEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY()
	UMaterialInterface* PreviousOverlayMaterial = nullptr;

	UPROPERTY()
	UMaterialInterface* AppliedOverlayMaterial = nullptr;
};

UCLASS()
class PUSH_API ATargetActor_GroundPick : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()
public:
	ATargetActor_GroundPick();

protected:
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ConfirmTargetingAndContinue() override;
	
	FVector GetTargetPoint() const;
	void SetTargetOptions(bool bTargetFriendly, bool bTargetEnemy = true);

private:

	bool bShouldTargetEnemy = true;
	bool bShouldTargetFriendly = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UDecalComponent* DecalComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	UMaterialInterface* TargetOverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetTraceRange = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float MaxDownwardTraceDistance = 10000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	float TargetAreaRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	TArray<FGroundPickOverlayEntry> OverlayEntries;

	void RefreshTargetOverlays();
	void ClearTargetOverlays();
	void ApplyTargetOverlay(UMeshComponent* MeshComponent);
	void RestoreTargetOverlay(int32 OverlayEntryIndex);
	bool HasOverlayEntryFor(UMeshComponent* MeshComponent) const;
	bool IsValidTargetActor(AActor* TargetActor, const IGenericTeamAgentInterface* OwnerTeamInterface) const;
	UMeshComponent* GetOverlayMeshComponent(AActor* TargetActor) const;

public:
	void SetTargetAreaRadius(const float NewRadius);
	
	FORCEINLINE void SetShouldDrawDebug(const bool bDrawDebug) { bShouldDrawDebug = bDrawDebug; }
	FORCEINLINE void SetTargetTraceRange(const float NewTraceRange) { TargetTraceRange = NewTraceRange; }
	FORCEINLINE void SetTargetOverlayMaterial(UMaterialInterface* NewOverlayMaterial) { TargetOverlayMaterial = NewOverlayMaterial; }
};
