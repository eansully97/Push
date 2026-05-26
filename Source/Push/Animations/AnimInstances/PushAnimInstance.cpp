// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPushAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());

	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UPushAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (OwnerCharacter)
	{
		CharacterSpeed = OwnerCharacter->GetVelocity().Length();
		
		FRotator BodyRotation = OwnerCharacter->GetActorRotation();
		FRotator BodyRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRotation, PreviousBodyRotation);
		
		PreviousBodyRotation = BodyRotation;
		YawSpeed = BodyRotationDelta.Yaw / DeltaTime;
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaTime, YawSpeedSmoothLerpSpeed);

		FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
		LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, BodyRotation);
	}

	if (MovementComponent)
	{
		bIsJumping = MovementComponent->IsFalling();
	}
}

void UPushAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}
