// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerController.h"

#include "PushPlayerCharacter.h"
#include "Push/Widgets/GameplayWidget.h"


void APushPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PushPlayerCharacter = Cast<APushPlayerCharacter>(InPawn);
	if (PushPlayerCharacter)
	{
		PushPlayerCharacter->ServerSideInit();
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
