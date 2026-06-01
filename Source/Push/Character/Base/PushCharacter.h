// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushCharacter.generated.h"

class UAIPerceptionStimuliSourceComponent;
class UWidgetComponent;
class UPushAbilitySystemComponent;
class UPushAttributeSet;
class UAttributeSet;

UCLASS()
class PUSH_API APushCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APushCharacter();

	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool UsesPlayerStateAbilitySystem() const;
	virtual bool ShouldApplyInitialEffects() const;
	
	/*
	 *	GAS
	 */
public:
	 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAbilitySystemComponent* GetActivePushAbilitySystemComponent() const;

	TArray<FPushInputActivatedAbilityDisplayData> GetDisplayInputActivatedAbilities() const;

	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	 
private:
	void ApplyMoveSpeed(float NewMoveSpeed);
	void SyncMoveSpeedFromAttribute();

	FDelegateHandle MoveSpeedChangedDelegateHandle;
	void InitializeAbilitySystem();
	UPushAbilitySystemComponent* ResolvePlayerStateAbilitySystemComponent() const;
	UPushAbilitySystemComponent* ResolveAbilitySystemComponent() const;
	UPushAbilitySystemComponent* ResolveDisplayAbilitySystemComponent() const;
	UPushAttributeSet* ResolveAttributeSet() const;
	void RegisterAttributeSetSubobjects(AActor* AttributeSetOwner) const;
	void BindChangeDelegates();
	void ClearChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 Count);
	void StunTagUpdated(const FGameplayTag Tag, int32 Count);
	void StealthTagUpdated(const FGameplayTag Tag, int32 Count);
	void AimingTagUpdated(const FGameplayTag Tag, int32 Count);
	void SetIsAiming(bool bIsAiming);
	virtual void OnAimStateChanged(bool bIsAiming);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;

	UPROPERTY()
	UPushAbilitySystemComponent* ActiveAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* ActiveAttributeSet;

	UPROPERTY()
	UPushAbilitySystemComponent* BoundAbilitySystemComponent;

	FDelegateHandle DeadTagDelegateHandle;
	FDelegateHandle StunTagDelegateHandle;
	FDelegateHandle StealthTagDelegateHandle;
	FDelegateHandle AimingTagDelegateHandle;

	/*
	*	OverheadWidget
	*/
public:
	void ConfigureOverheadWidget();
	void SetOverheadWidgetVisibility(bool Hidden);
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Widgets")
	UWidgetComponent* OverheadWidgetComponent;
	
	FTimerHandle OverheadWidgetVisibilityUpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityCheckInterval = .12f;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityRangeSq = 1000000.f;

protected:
	void UpdateOverheadWidgetVisibility();

private:
	void SetStatusGaugeEnabled(bool bEnabled);
	
	/*
	*	Death and Respawn
	*/
public:
	bool IsDead() const;
	void RespawnImmediately();
	
private:
	void StartDeathSequence();
	void PlayDeathAnimation();
	void Respawn();
	void DeathMontageFinished();
	void SetRagdollEnabled(bool bEnabled);
	void PrepareMovementAndCollisionForDeath();
	void CacheAliveCapsuleCollisionState();
	void SetCapsuleCollisionForDeath(ECollisionEnabled::Type NewCollisionEnabled);
	void DisableCapsuleCollisionForDeath();
	void RestoreAliveCapsuleCollision();

	virtual void OnDead();
	virtual void OnRespawn();

	FTimerHandle DeathMontageTimerHandle;
	FTransform RelativeMeshTransform;
	FName AliveCapsuleCollisionProfileName;
	ECollisionEnabled::Type AliveCapsuleCollisionEnabled = ECollisionEnabled::QueryAndPhysics;
	ECollisionChannel AliveCapsuleObjectType = ECC_Pawn;
	FCollisionResponseContainer AliveCapsuleCollisionResponses;
	bool bCapsuleCollisionModifiedForDeath = false;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f;

	/*
	*	Stun
	*/
private:
	virtual void OnStun();
	virtual void OnStunRemoved();

	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	/*
	*	Stealth
	*/
private:
	virtual void OnStealth();
	virtual void OnStealthRemoved();
	bool IsInStealth() const;
	
	/*
	*	Generic Team Agent Interface
	*/
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID();

	/*
	*	AI
	*/
private:
	void SetAIPerceptionStimuliSourceEnabled(bool bEnabled);
	
	UPROPERTY()
	UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
};
