// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerController.h"

#include "Push/Player/Characters/PushPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Push/Widgets/HUD/GameplayWidget.h"


void APushPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PushPlayerCharacter = Cast<APushPlayerCharacter>(InPawn);
	if (PushPlayerCharacter)
	{
		PushPlayerCharacter->ServerSideInit();
		PushPlayerCharacter->SetGenericTeamId(TeamID);
	}
}

void APushPlayerController::AcknowledgePossession(APawn* InPawn)
{
	Super::AcknowledgePossession(InPawn);

	PushPlayerCharacter = Cast<APushPlayerCharacter>(InPawn);
	if (PushPlayerCharacter)
	{
		PushPlayerCharacter->ClientSideInit();
		SpawnGameplayWidget();
	}
}

void APushPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId APushPlayerController::GetGenericTeamId() const
{
	return TeamID;
}

void APushPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamID);
}

void APushPlayerController::SpawnGameplayWidget()
{
	if (!IsLocalPlayerController())
		return;

	GameplayWidget = CreateWidget<UGameplayWidget>(this, GameplayWidgetClass);

	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
}
