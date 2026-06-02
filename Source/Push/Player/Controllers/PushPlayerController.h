// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "PushPlayerController.generated.h"

class UGameplayWidget;
class APushPlayerCharacter;

/**
 * 
 */
UCLASS()
class PUSH_API APushPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	APushPlayerController();

	virtual void SpawnPlayerCameraManager() override;

	// Only called on server
	virtual void OnPossess(APawn* InPawn) override;
	// Only called on client and listening server
	virtual void AcknowledgePossession(APawn* InPawn) override;

	/*
	 *	Generic Team Agent Interface
	 */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
private:
	void SpawnGameplayWidget();
	
	UPROPERTY()
	APushPlayerCharacter* PushPlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameplayWidget> GameplayWidgetClass;

	UPROPERTY()
	UGameplayWidget* GameplayWidget;

	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
