// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "BTTask_SendInputToASC.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UBTTask_SendInputToASC : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Ability")
	EAbilityInputID InputID;
};
