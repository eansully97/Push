// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
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

	UFUNCTION()
	void TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

	UFUNCTION()
	void TargetForgotten(AActor* TargetActor);

	const UObject* GetCurrentTarget() const;
	void SetCurrentTarget(AActor* NewTarget);
	void ForgetActorIfDead(AActor* ActorToForget);

	AActor* GetNextPerceivedActor() const;
};

