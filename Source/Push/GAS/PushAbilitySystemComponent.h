// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "PushAbilitySystemComponent.generated.h"


UCLASS()
class PUSH_API UPushAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	void ApplyInitialEffects();
	void GiveInitialAbilities();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>> BasicAbilities;
};
