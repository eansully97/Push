// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "PushCharacter.generated.h"

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
protected:
	 virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	 
private:
	void BindChangeDelegates();
	void DeathTagUpdated(FGameplayTag Tag, int32 Count);
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;

	/*
	*	OverheadWidget
	*/
public:
	void ConfigureOverheadWidget();
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Widgets")
	UWidgetComponent* OverheadWidgetComponent;
	
	FTimerHandle OverheadWidgetVisibilityUpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityCheckInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityRangeSq = 1000000.f;

	void UpdateOverheadWidgetVisibility();
	void SetStatusGaugeEnabled(bool bEnabled);
	
	/*
	*	Death and Respawn
	*/
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
	*	Generic Team Agent Interface
	*/
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
