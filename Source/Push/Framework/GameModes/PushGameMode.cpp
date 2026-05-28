// Fill out your copyright notice in the Description page of Project Settings.


#include "PushGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Push/Player/States/PushPlayerState.h"

APushGameMode::APushGameMode()
{
	PlayerStateClass = APushPlayerState::StaticClass();
}

APlayerController* APushGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	const FGenericTeamId TeamID = GetTeamIDForPlayer(NewPlayerController);
	if (IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController))
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamID);
	}
	
	NewPlayerController->StartSpot = FindNextStartSpotForTeam(TeamID);
	return NewPlayerController;
}

FGenericTeamId APushGameMode::GetTeamIDForPlayer(const APlayerController* PlayerController) const
{
	static int PlayerCount = 0;
	++PlayerCount;
	return FGenericTeamId(PlayerCount % 2);
}

AActor* APushGameMode::FindNextStartSpotForTeam(const FGenericTeamId InTeamID)
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(InTeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}
	return nullptr;
}
