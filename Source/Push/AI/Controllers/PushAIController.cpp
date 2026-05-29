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
	
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2200.f;

	SightConfig->SetMaxAge(5.f);

	SightConfig->PeripheralVisionAngleDegrees = 180.f;

	AIPerceptionComponent->ConfigureSense(*SightConfig);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::TargetPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ThisClass::TargetForgotten);
}

void APushAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UnbindPawnDeathTagEvents();

	if (IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(InPawn))
	{
		SetGenericTeamId(PawnTeamInterface->GetGenericTeamId());
		ClearAndDisableAllSenses();
		EnableAllSenses();
	}

	BindPawnDeathTagEvents(InPawn);
}

void APushAIController::OnUnPossess()
{
	UnbindPawnDeathTagEvents();
	ClearRememberedTarget();
	UnbindTargetTagEvents();
	GetWorldTimerManager().ClearTimer(RememberedTargetTimerHandle);

	Super::OnUnPossess();
}

void APushAIController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!BehaviorTree)
		return;
	RunBehaviorTree(BehaviorTree);
}

void APushAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPawnDeathTagEvents();
	ClearRememberedTarget();
	UnbindTargetTagEvents();
	GetWorldTimerManager().ClearTimer(RememberedTargetTimerHandle);

	Super::EndPlay(EndPlayReason);
}

void APushAIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (!TargetActor)
		return;

	const bool bWasCurrentTarget = GetCurrentTarget() == TargetActor;
	const bool bWasRememberedTarget = RememberedTarget.Get() == TargetActor;
	if (ForgetActorIfDead(TargetActor))
	{
		if (bWasCurrentTarget || bWasRememberedTarget)
		{
			SetCurrentTarget(GetNextPerceivedActor());
		}
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (IsStealthedTarget(TargetActor))
		{
			HideTargetForStealth(TargetActor);
		}
		else if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		if (IsStealthedTarget(TargetActor))
		{
			HideTargetForStealth(TargetActor);
		}
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

	if (RememberedTarget.Get() == TargetActor)
	{
		ClearRememberedTarget();
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
	if (NewTarget && ForgetActorIfDead(NewTarget))
	{
		NewTarget = nullptr;
	}

	if (NewTarget && IsStealthedTarget(NewTarget))
	{
		HideTargetForStealth(NewTarget);
		return;
	}

	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		ClearRememberedTarget();
		UnbindTargetTagEvents();
		return;
	}

	ClearRememberedTarget();
	UnbindTargetTagEvents();
	
	if (NewTarget)
	{
		BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, NewTarget);
		BindTargetTagEvents(NewTarget);
	}
	else
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

void APushAIController::ClearCurrentTargetBlackboardValue()
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetBlackboardKeyName);
	}
}

bool APushAIController::IsDeadTarget(AActor* ActorToCheck) const
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToCheck);
	return ActorASC && ActorASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead);
}

bool APushAIController::IsStealthedTarget(AActor* ActorToCheck) const
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToCheck);
	return ActorASC && ActorASC->HasMatchingGameplayTag(PushGameplayTags::Status_Stealth);
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

bool APushAIController::ForgetActorIfDead(AActor* ActorToForget)
{
	if (!IsDeadTarget(ActorToForget))
		return false;

	ForceForgetActor(ActorToForget);
	if (RememberedTarget.Get() == ActorToForget)
	{
		ClearRememberedTarget();
	}
	return true;
}

void APushAIController::HideTargetForStealth(AActor* TargetActor)
{
	if (!TargetActor || IsDeadTarget(TargetActor))
		return;

	if (!IsStealthedTarget(TargetActor))
	{
		RestoreRememberedTargetIfValid();
		return;
	}

	RememberHiddenTarget(TargetActor);

	if (GetCurrentTarget() == TargetActor)
	{
		ClearCurrentTargetBlackboardValue();
	}
}

void APushAIController::RememberHiddenTarget(AActor* TargetActor)
{
	if (!TargetActor)
		return;

	RememberedTarget = TargetActor;
	BindTargetTagEvents(TargetActor);

	GetWorldTimerManager().ClearTimer(RememberedTargetTimerHandle);
	GetWorldTimerManager().SetTimer(
		RememberedTargetTimerHandle,
		this,
		&ThisClass::ExpireRememberedTarget,
		GetTargetMemoryDuration(),
		false);
}

void APushAIController::ClearRememberedTarget()
{
	AActor* PreviousRememberedTarget = RememberedTarget.Get();
	GetWorldTimerManager().ClearTimer(RememberedTargetTimerHandle);
	RememberedTarget.Reset();

	if (PreviousRememberedTarget && TargetTagEventActor.Get() == PreviousRememberedTarget && GetCurrentTarget() != PreviousRememberedTarget)
	{
		UnbindTargetTagEvents();
	}
}

void APushAIController::ExpireRememberedTarget()
{
	ClearRememberedTarget();
}

void APushAIController::RestoreRememberedTargetIfValid()
{
	AActor* TargetToRestore = RememberedTarget.Get();
	if (!TargetToRestore || GetCurrentTarget() || IsDeadTarget(TargetToRestore) || IsStealthedTarget(TargetToRestore))
		return;

	SetCurrentTarget(TargetToRestore);
}

float APushAIController::GetTargetMemoryDuration() const
{
	return SightConfig ? SightConfig->GetMaxAge() : 5.f;
}

AActor* APushAIController::GetNextPerceivedActor()
{
	if (!AIPerceptionComponent)
		return nullptr;

	TArray<AActor*> PerceivedActors;
	AIPerceptionComponent->GetPerceivedHostileActors(PerceivedActors);

	for (AActor* PerceivedActor : PerceivedActors)
	{
		if (ForgetActorIfDead(PerceivedActor) || IsStealthedTarget(PerceivedActor))
		{
			continue;
		}

		return PerceivedActor;
	}
	return nullptr;
}

void APushAIController::BindTargetTagEvents(AActor* TargetActor)
{
	if (!TargetActor || TargetTagEventActor.Get() == TargetActor)
		return;

	UnbindTargetTagEvents();

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
		return;

	TargetTagEventActor = TargetActor;
	TargetDeadTagDelegateHandle = TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).AddUObject(this, &ThisClass::TargetStateTagUpdated);
	TargetStealthTagDelegateHandle = TargetASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Stealth).AddUObject(this, &ThisClass::TargetStateTagUpdated);
}

void APushAIController::UnbindTargetTagEvents()
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

	ResetTargetTagHandles();
}

void APushAIController::ResetTargetTagHandles()
{
	TargetTagEventActor.Reset();
	TargetDeadTagDelegateHandle.Reset();
	TargetStealthTagDelegateHandle.Reset();
}

void APushAIController::TargetStateTagUpdated(const FGameplayTag Tag, int32 Count)
{
	AActor* TargetActor = TargetTagEventActor.Get();
	if (!TargetActor)
		return;

	if (Tag == PushGameplayTags::Status_Dead && Count != 0)
	{
		const bool bWasCurrentTarget = GetCurrentTarget() == TargetActor;
		const bool bWasRememberedTarget = RememberedTarget.Get() == TargetActor;
		if (ForgetActorIfDead(TargetActor) && (bWasCurrentTarget || bWasRememberedTarget))
		{
			SetCurrentTarget(GetNextPerceivedActor());
		}
		return;
	}

	if (Tag == PushGameplayTags::Status_Stealth)
	{
		if (Count != 0)
		{
			HideTargetForStealth(TargetActor);
		}
		else if (RememberedTarget.Get() == TargetActor)
		{
			RestoreRememberedTargetIfValid();
		}
	}
}

void APushAIController::ClearAndDisableAllSenses()
{
	AIPerceptionComponent->AgeStimuli(TNumericLimits<float>::Max());

	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}

	ClearRememberedTarget();
	UnbindTargetTagEvents();
	ClearCurrentTargetBlackboardValue();
}

void APushAIController::EnableAllSenses()
{
	for (auto SenseConfigIt = AIPerceptionComponent->GetSensesConfigIterator(); SenseConfigIt; ++SenseConfigIt)
	{
		AIPerceptionComponent->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}

void APushAIController::BindPawnDeathTagEvents(APawn* PawnToBind)
{
	UAbilitySystemComponent* PawnASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PawnToBind);
	if (!PawnASC)
	{
		return;
	}

	PawnDeathTagASC = PawnASC;
	PawnDeadTagDelegateHandle = PawnASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead)
		.AddUObject(this, &ThisClass::PawnDeadTagUpdated);
}

void APushAIController::UnbindPawnDeathTagEvents()
{
	if (UAbilitySystemComponent* PawnASC = PawnDeathTagASC.Get())
	{
		if (PawnDeadTagDelegateHandle.IsValid())
		{
			PawnASC->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).Remove(PawnDeadTagDelegateHandle);
		}
	}

	PawnDeathTagASC.Reset();
	PawnDeadTagDelegateHandle.Reset();
}

void APushAIController::PawnDeadTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		if (UBrainComponent* ActiveBrainComponent = GetBrainComponent())
		{
			ActiveBrainComponent->StopLogic("Dead");
		}
		ClearAndDisableAllSenses();
		bIsPawnDead = true;
	}
	else
	{
		if (UBrainComponent* ActiveBrainComponent = GetBrainComponent())
		{
			ActiveBrainComponent->StartLogic();
		}
		EnableAllSenses();
		bIsPawnDead = false;
	}
}
