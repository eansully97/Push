// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "PushCharacter.generated.h"

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

protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;
};
