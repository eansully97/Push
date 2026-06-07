// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "AN_SendTargetGroup.generated.h"

/**
 * 
 */
UCLASS()
class PUSH_API UAN_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TEnumAsByte<ETeamAttitude::Type> TargetTeam{ETeamAttitude::Hostile};

	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TArray<FName> TargetSocketNames;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float SphereRadius{50.f};

	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bIgnoreOwner{true};

	UPROPERTY(EditAnywhere, Category = "Targeting")
	bool bDrawDebug{false};

	UPROPERTY(EditAnywhere, Category = "Gameplay Cue")
	FGameplayTagContainer GameplayCueTriggerTags;

	void SendLocalGameplayCue(const FHitResult& HitResult, USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const;
};
