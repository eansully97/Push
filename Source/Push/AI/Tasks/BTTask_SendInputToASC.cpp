// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_SendInputToASC.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_SendInputToASC::ExecuteTask(UBehaviorTreeComponent& Comp, uint8* NodeMemory)
{
	if (AAIController* OwnerAIC = Comp.GetAIOwner())
	{
		if (UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerAIC->GetPawn()))
		{
			OwnerASC->PressInputID(static_cast<int32>(InputID));
			OwnerASC->ReleaseInputID(static_cast<int32>(InputID));
			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}
