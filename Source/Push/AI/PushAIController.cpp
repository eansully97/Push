// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Push/PushGameplayTags.h"

APushAIController::APushAIController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1200.f;

	SightConfig->SetMaxAge(5.f);

	SightConfig->PeripheralVisionAngleDegrees = 180.f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ThisClass::TargetForgotten);
}

void APushAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(InPawn))
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}

	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn());
	if (PawnASC)
	{
		PawnASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).AddUObject(this, &ThisClass::PawnDeadTagUpdated);
		PawnASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Stun).AddUObject(this, &ThisClass::PawnStunTagUpdated);
	}
}

void APushAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!BehaviorTree)
		return;
	RunBehaviorTree(BehaviorTree);
}

void APushAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (!TargetActor)
		return;

	if (ForgetActorIfInvalid(TargetActor))
	{
		if (GetCurrentTarget() == TargetActor)
		{
			SetCurrentTarget(GetNextPerceivedActor());
		}
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		ForgetActorIfInvalid(TargetActor);
	}
}

void APushAIController::TargetForgotten(AActor* TargetActor)
{
	if (!TargetActor)
		return;

	if (GetCurrentTarget() == TargetActor)
	{
		SetCurrentTarget(GetNextPerceivedActor());
	}
}

AActor* APushAIController::GetCurrentTarget() const
{
	if (const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		return Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName));
	}
	return nullptr;
}

void APushAIController::SetCurrentTarget(AActor* NewTarget)
{
	if (NewTarget && ForgetActorIfInvalid(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (GetCurrentTarget() == NewTarget && TargetTagEventActor.Get() == NewTarget)
	{
		return;
	}

	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		UnbindTargetInvalidTagEvents();
		return;
	}

	UnbindTargetInvalidTagEvents();
	
	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
		BindTargetInvalidTagEvents(NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

bool APushAIController::IsInvalidTargetActor(AActor* ActorToCheck) const
{
	if (!ActorToCheck)
		return true;

	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToCheck);
	if (!ActorASC)
		return false;

	return ActorASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead)
		|| ActorASC->HasMatchingGameplayTag(PushGameplayTags::Status_Stealth);
}

void APushAIController::ForceForgetActor(AActor* ActorToForget)
{
	if (!ActorToForget || !AIPerceptionComponent)
		return;

	for (UAIPerceptionComponent::TActorPerceptionContainer::TIterator Iter = AIPerceptionComponent->GetPerceptualDataIterator(); Iter; ++Iter)
	{
		if (Iter->Key != ActorToForget)
		{
			continue;
		}

		for (FAIStimulus& Stimulus : Iter->Value.LastSensedStimuli)
		{
			Stimulus.SetStimulusAge(TNumericLimits<float>::Max());
		}
	}
}

bool APushAIController::ForgetActorIfInvalid(AActor* ActorToForget)
{
	if (!IsInvalidTargetActor(ActorToForget))
		return false;

	ForceForgetActor(ActorToForget);
	return true;
}

AActor* APushAIController::GetNextPerceivedActor()
{
	if (!AIPerceptionComponent)
		return nullptr;

	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetPerceivedHostileActors(PerceivedActors);

	for (AActor* PerceivedActor : PerceivedActors)
	{
		if (ForgetActorIfInvalid(PerceivedActor))
		{
			continue;
		}

		return PerceivedActor;
	}
	return nullptr;
}

void APushAIController::BindTargetInvalidTagEvents(AActor* TargetActor)
{
	if (!TargetActor)
		return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
		return;

	TargetTagEventActor = TargetActor;
	TargetDeadTagDelegateHandle = TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).AddUObject(this, &ThisClass::TargetInvalidTagUpdated);
	TargetStealthTagDelegateHandle = TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Stealth).AddUObject(this, &ThisClass::TargetInvalidTagUpdated);
}

void APushAIController::UnbindTargetInvalidTagEvents()
{
	AActor* TargetActor = TargetTagEventActor.Get();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC)
	{
		if (TargetDeadTagDelegateHandle.IsValid())
		{
			TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).Remove(TargetDeadTagDelegateHandle);
		}
		if (TargetStealthTagDelegateHandle.IsValid())
		{
			TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Stealth).Remove(TargetStealthTagDelegateHandle);
		}
	}

	ResetTargetInvalidTagHandles();
}

void APushAIController::ResetTargetInvalidTagHandles()
{
	TargetTagEventActor.Reset();
	TargetDeadTagDelegateHandle.Reset();
	TargetStealthTagDelegateHandle.Reset();
}

void APushAIController::TargetInvalidTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count == 0)
		return;

	AActor* TargetActor = TargetTagEventActor.Get();
	if (!TargetActor || GetCurrentTarget() != TargetActor)
		return;

	ForceForgetActor(TargetActor);
	SetCurrentTarget(GetNextPerceivedActor());
}

void APushAIController::ClearAndDisableAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());
	SetCurrentTarget(nullptr);

	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}
}

void APushAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void APushAIController::PawnStunTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (bIsPawnDead)
		return;

	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Stun");
	}
	else
	{
		GetBrainComponent()->StartLogic();
	}
}

void APushAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		GetBrainComponent()->StopLogic("Dead");
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}
	else
	{
		GetBrainComponent()->StartLogic();
		EnableAllSenses();
		bIsPawnDead = false;
	}
}
