// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "PushCharacter.generated.h"

class UWidgetComponent;
class UPushAbilitySystemComponent;
class UPushAttributeSet;

UCLASS()
class PUSH_API APushCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APushCharacter();

	void ServerSideInit();
	void ClientSideInit();
	bool IsLocallyControlledByPlayer() const;

	void ConfigureOverheadWidget();

protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void BeginPlay() override;

	//Only Called On Server
	virtual void PossessedBy(AController* NewController) override;

private:

	/*
	 *	GAS
	 */
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;

	/*
	*	OverheadWidget
	*/
	UPROPERTY(VisibleDefaultsOnly, Category = "Widgets")
	UWidgetComponent* OverheadWidgetComponent;
	
	FTimerHandle OverheadWidgetVisibilityUpdateTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityCheckInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "OverheadWidget")
	float OverheadWidgetVisibilityRangeSq = 1000000.f;

	void UpdateOverheadWidgetVisibility();
};
