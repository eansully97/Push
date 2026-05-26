// Fill out your copyright notice in the Description page of Project Settings.


#include "AN_SendTargetGroup.h"

#include "AbilitySystemBlueprintLibrary.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	if (TargetSocketNames.Num() <= 1)
		return;

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		return;

	FGameplayEventData EventData;

	for (int i = 1; i < TargetSocketNames.Num(); i++)
	{
		// Heap Allocation
		FGameplayAbilityTargetData_LocationInfo* LocationInfo = new FGameplayAbilityTargetData_LocationInfo();

		FVector StartLoc = MeshComp->GetSocketLocation(TargetSocketNames[i-1]);
		FVector EndLoc = MeshComp->GetSocketLocation(TargetSocketNames[i]);

		LocationInfo->SourceLocation.LiteralTransform.SetLocation(StartLoc);
		LocationInfo->TargetLocation.LiteralTransform.SetLocation(EndLoc);

		EventData.TargetData.Add(LocationInfo);
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, EventData);
}
