// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "GenericTeamAgentInterface.h"
#include "Push/Widgets/Gauges/ValueGauge.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"

void UOverheadWidget::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent) const
{
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UPushAttributeSet::GetHealthAttribute(), UPushAttributeSet::GetMaxHealthAttribute());
		ManaBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UPushAttributeSet::GetManaAttribute(), UPushAttributeSet::GetMaxManaAttribute());

		SetObservedActor(AbilitySystemComponent->GetAvatarActor());
	}
}

void UOverheadWidget::SetObservedActor(AActor* ObservedActor) const
{
	if (!ObservedActor || !HealthBar)
	{
		return;
	}

	const APlayerController* LocalPC = GetOwningPlayer();
	const APawn* LocalPawn = LocalPC ? LocalPC->GetPawn() : nullptr;

	const IGenericTeamAgentInterface* LocalTeamAgent =
		Cast<IGenericTeamAgentInterface>(LocalPawn);

	const IGenericTeamAgentInterface* ObservedTeamAgent =
		Cast<IGenericTeamAgentInterface>(ObservedActor);

	if (!LocalTeamAgent || !ObservedTeamAgent)
	{
		HealthBar->SetBarColor(FLinearColor::White);
		return;
	}

	const FGenericTeamId LocalTeam = LocalTeamAgent->GetGenericTeamId();
	const FGenericTeamId ObservedTeam = ObservedTeamAgent->GetGenericTeamId();

	const bool bIsAlly = LocalTeam == ObservedTeam;

	HealthBar->SetBarColor(bIsAlly ? FLinearColor::Green : FLinearColor::Red);
}
