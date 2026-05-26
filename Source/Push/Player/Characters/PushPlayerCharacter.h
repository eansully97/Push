// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/Character/Base/PushCharacter.h"
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
	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

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
	
	void SetInputEnabledFromPlayerController(bool bEnabled);

	/*
	*	Death and Respawn
	*/
private:
	virtual void OnDead() override;
	virtual void OnRespawn() override;

	/*
	*	Stun
	*/
private:
	virtual void OnStun() override;
	virtual void OnStunRemoved() override;

	/*
	*	Stealth
	*/
private:
	virtual void OnStealth() override;
	virtual void OnStealthRemoved() override;
};
