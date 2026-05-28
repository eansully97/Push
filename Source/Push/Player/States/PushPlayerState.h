// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "PushPlayerState.generated.h"

class UPushAbilitySystemComponent;
class UPushAttributeSet;
class UAbilitySystemComponent;

UCLASS()
class PUSH_API APushPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APushPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UPushAbilitySystemComponent* GetPushAbilitySystemComponent() const;
	UPushAttributeSet* GetPushAttributeSet() const;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;
};
