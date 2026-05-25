// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "PushCharacter.generated.h"

class UAIPerceptionStimuliSourceComponent;
class UWidgetComponent;
class UPushAbilitySystemComponent;
class UPushAttributeSet;

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
	virtual void PossessedBy(AController* NewController) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/*
	 *	GAS
	 */
public:
	 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);
	 
private:
	void BindChangeDelegates();
	void DeathTagUpdated(const FGameplayTag Tag, int32 Count);
	void StunTagUpdated(const FGameplayTag Tag, int32 Count);
	void StealthTagUpdated(const FGameplayTag Tag, int32 Count);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;

	/*
	*	OverheadWidget
	*/
public:
	void ConfigureOverheadWidget();
	void SetOverheadWidgetVisibility(bool bVisible);
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Widgets")
	UWidgetComponent* OverheadWidgetComponent;
	
	FTimerHandle OverheadWidgetVisibilityUpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityCheckInterval = .12f;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityRangeSq = 1000000.f;

	void UpdateOverheadWidgetVisibility();
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

	virtual void OnDead();
	virtual void OnRespawn();

	FTimerHandle DeathMontageTimerHandle;
	FTransform RelativeMeshTransform;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageFinishTimeShift = -0.8f;

	/*
	*	Stun
	*/
private:
	virtual void OnStun();
	virtual void StunRemoved();

	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;

	/*
	*	Stealth
	*/
private:
	virtual void OnStealth();
	virtual void StealthRemoved();
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

