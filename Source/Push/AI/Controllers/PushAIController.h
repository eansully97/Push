// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "PushAIController.generated.h"

class UAbilitySystemComponent;

UCLASS()
class PUSH_API APushAIController : public AAIController
{
	GENERATED_BODY()

public:
	APushAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void BeginPlay() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	FName TargetBlackboardKeyName = "Target";
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI Behavior")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleDefaultsOnly, Category = "AI Behavior")
	bool bIsPawnDead = false;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* TargetActor);

	AActor* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);
	void ClearCurrentTargetBlackboardValue();

	bool IsDeadTarget(AActor* ActorToCheck) const;
	bool IsStealthedTarget(AActor* ActorToCheck) const;
	void ForceForgetActor(AActor* ActorToForget);
	bool ForgetActorIfDead(AActor* ActorToForget);
	void HideTargetForStealth(AActor* TargetActor);
	void RememberHiddenTarget(AActor* TargetActor);
	void ClearRememberedTarget();
	void ExpireRememberedTarget();
	void RestoreRememberedTargetIfValid();
	float GetTargetMemoryDuration() const;

	AActor* GetNextPerceivedActor();

	void BindTargetTagEvents(AActor* TargetActor);
	void UnbindTargetTagEvents();
	void ResetTargetTagHandles();
	void TargetStateTagUpdated(const FGameplayTag Tag, int32 Count);

	void ClearAndDisableAllSenses();
	void EnableAllSenses();
	
	void BindPawnDeathTagEvents(APawn* PawnToBind);
	void UnbindPawnDeathTagEvents();
	void RefreshPawnDeathState();
	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count);

	TWeakObjectPtr<AActor> TargetTagEventActor;
	FDelegateHandle TargetDeadTagDelegateHandle;
	FDelegateHandle TargetStealthTagDelegateHandle;

	TWeakObjectPtr<UAbilitySystemComponent> PawnDeathTagASC;
	FDelegateHandle PawnDeadTagDelegateHandle;

	TWeakObjectPtr<AActor> RememberedTarget;
	FTimerHandle RememberedTargetTimerHandle;
};

