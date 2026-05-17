// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/PushCharacter.h"
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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	UCameraComponent* ViewCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* GameplayInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

public:
	void HandleLookInput(const FInputActionValue& ActionValue);
	void HandleMoveInput(const FInputActionValue& ActionValue);

	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

};
