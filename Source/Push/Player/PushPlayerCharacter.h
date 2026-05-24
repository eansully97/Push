// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/Character/PushCharacter.h"
#include "PushPlayerCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PUSH_API APushPlayerCharacter : public APushCharacter
{
	GENERATED_BODY()

public:
	APushPlayerCharacter();
	void CacheMaterials();
	
	UFUNCTION(BlueprintCallable)
	void SetCachedMaterials();

	FVector GetMovementInputDirection() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	/*
	*	Components
	*/
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleDefaultsOnly, Category = "Materials")
	TArray<UMaterialInterface*> CachedMaterials;
 
	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

	/*
	*	Input
	*/
private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* GameplayInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EAbilityInputID, UInputAction*> AbilityInputActions;
	
	void HandleLookInput(const FInputActionValue& ActionValue);
	void HandleMoveInput(const FInputActionValue& ActionValue);
	void HandleAbilityInput(const FInputActionValue& ActionValue, EAbilityInputID AbilityInputID);

	/*
	*	Death and Respawn
	*/
	virtual void OnDeath();
	virtual void OnRespawn();
};
