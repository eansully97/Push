// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "PushPlayerState.generated.h"

class UPushAbilitySystemComponent;
class UPushAttributeSet;
class UPushHeroAttributeSet;
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
	UPushHeroAttributeSet* GetPushHeroAttributeSet() const;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Gameplay Ability")
	UPushAbilitySystemComponent* PushAbilitySystemComponent;

	UPROPERTY()
	UPushAttributeSet* PushAttributeSet;

	UPROPERTY()
	UPushHeroAttributeSet* HeroAttributeSet;
};
