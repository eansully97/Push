// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Push/PushGameplayAbilityTypes.h"
#include "Push/Character/Base/PushCharacter.h"
#include "PushPlayerCharacter.generated.h"

class UPushHeroAttributeSet;
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

	FVector GetMovementInputDirection() const;
	FVector GetLookRightDirection() const;
	FVector GetLookForwardDirection() const;
	FVector GetMoveForwardDirection() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual bool UsesPlayerStateAbilitySystem() const override;

private:
	/*
	*	Components
	*/
	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleDefaultsOnly, Category = "View")
	UCameraComponent* ViewCamera;

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

	bool bSuppressBasicAttackUntilRelease = false;
	bool bSuppressSecondaryAttackUntilRelease = false;
	
	void HandleLookInput(const FInputActionValue& ActionValue);
	void HandleMoveInput(const FInputActionValue& ActionValue);
	void HandleAbilityInput(const FInputActionValue& ActionValue, EAbilityInputID AbilityInputID);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);
	bool IsWellFormedClientGameplayEvent(const FGameplayTag& EventTag, const FGameplayEventData& EventData) const;
	bool IsPossessedByPlayerController() const;
	bool CanProcessClientGameplayEvent(const FGameplayTag& EventTag, const FGameplayEventData& EventData) const;
	bool IsClientGameplayEventThrottled(const FGameplayTag& EventTag);

	void SetInputEnabledFromPlayerController(bool bEnabled);

	TMap<FGameplayTag, double> LastAcceptedClientGameplayEventTimes;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability", meta = (ClampMin = "0.0"))
	float ClientGameplayEventThrottleSeconds = 0.05f;

	/*
	*	GAS
	*/
private:
	virtual void OnAimStateChanged(bool bIsAiming) override;

	UPROPERTY()
	UPushHeroAttributeSet* HeroAttributeSet;

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

	/*
	*	Camera View
	*/
private:
	UPROPERTY(EditDefaultsOnly, Category = "View")
	FVector CameraAimLocalOffset{FVector()};

	UPROPERTY(EditDefaultsOnly, Category = "View")
	float CameraLerpSpeed = 20.f;

	FTimerHandle CameraLerpTimerHandle;
	int32 CameraLerpGeneration = 0;

	void LerpCameraToLocalOffset(const FVector& Goal);
	void TickCameraLocalOffset(FVector Goal, int32 LerpGeneration);
	
};
