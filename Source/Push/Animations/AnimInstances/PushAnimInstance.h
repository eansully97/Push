// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"
#include "PushAnimInstance.generated.h"

class UAbilitySystemComponent;
class UCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class UPushAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	

private:
	void OwnerAimingTagUpdated(const FGameplayTag Tag, int32 Count);
	void ClearOwnerAimingTagBinding();
	
	UPROPERTY()
	ACharacter* OwnerCharacter;

	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;

	UPROPERTY()
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	FDelegateHandle OwnerAimingTagDelegateHandle;

	UPROPERTY()
	float CharacterSpeed;
	float ForwardSpeed;
	float RightSpeed;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float YawSpeedSmoothLerpSpeed = 1.f;
	float SmoothedYawSpeed;
	float YawSpeed;
	
	UPROPERTY()
	FRotator PreviousBodyRotation;

	UPROPERTY()
	FRotator LookRotationOffset;

	UPROPERTY()
	bool bIsJumping;

	UPROPERTY()
	bool bIsAiming;

public:
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetCharacterSpeed() const
	{
		return CharacterSpeed;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetCharacterForwardSpeed() const
	{
		return ForwardSpeed;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetCharacterRightSpeed() const
	{
		return RightSpeed;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const
	{
		return CharacterSpeed != 0;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const
	{
		return CharacterSpeed == 0;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetYawSpeed() const
	{
		return YawSpeed;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSmoothedYawSpeed() const
	{
		return SmoothedYawSpeed;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsJumping() const
	{
		return bIsJumping;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsOnGround() const
	{
		return !bIsJumping;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool GetIsAiming() const
	{
		return bIsAiming;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool ShouldDoFullBody() const
	{
		return GetCharacterSpeed() <= 0 && !GetIsAiming();
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetYawLookOffset() const
	{
		return LookRotationOffset.Yaw;
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetPitchLookOffset() const
	{
		return LookRotationOffset.Pitch;
	}
};
