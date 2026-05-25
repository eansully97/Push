// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameplayTagContainer.h"
#include "PushAIController.generated.h"

UCLASS()
class PUSH_API APushAIController : public AAIController
{
	GENERATED_BODY()

public:
	APushAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	
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

	TWeakObjectPtr<AActor> TargetTagEventActor;
	FDelegateHandle TargetDeadTagDelegateHandle;
	FDelegateHandle TargetStunTagDelegateHandle;
	FDelegateHandle TargetStealthTagDelegateHandle;

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* TargetActor);

	AActor* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);
	bool IsInvalidTargetActor(AActor* ActorToCheck) const;
	void ForceForgetActor(AActor* ActorToForget);
	bool ForgetActorIfInvalid(AActor* ActorToForget);

	AActor* GetNextPerceivedActor();

	void BindTargetInvalidTagEvents(AActor* TargetActor);
	void UnbindTargetInvalidTagEvents();
	void ResetTargetInvalidTagHandles();
	void TargetInvalidTagUpdated(const FGameplayTag Tag, int32 Count);

	void ClearAndDisableAllSenses();
	void EnableAllSenses();
	
	void PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count);
	void PawnStunTagUpdated(const FGameplayTag Tag, int32 Count);
};

