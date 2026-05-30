// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAnimInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Push/PushGameplayTags.h"

void UPushAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ClearOwnerAimingTagBinding();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	MovementComponent = nullptr;
	bIsAiming = false;

	if (OwnerCharacter)
	{
		MovementComponent = OwnerCharacter->GetCharacterMovement();
	}

	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
	if (OwnerAbilitySystemComponent)
	{
		bIsAiming = OwnerAbilitySystemComponent->HasMatchingGameplayTag(PushGameplayTags::Status_Aiming);
		OwnerAimingTagDelegateHandle = OwnerAbilitySystemComponent
			->RegisterGameplayTagEvent(PushGameplayTags::Status_Aiming)
			.AddUObject(this, &ThisClass::OwnerAimingTagUpdated);
	}
}

void UPushAnimInstance::NativeUninitializeAnimation()
{
	ClearOwnerAimingTagBinding();
	Super::NativeUninitializeAnimation();
}

void UPushAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (OwnerCharacter)
	{
		FVector Velocity = OwnerCharacter->GetVelocity();
		CharacterSpeed = Velocity.Length();
		
		FRotator BodyRotation = OwnerCharacter->GetActorRotation();
		FRotator BodyRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRotation, PreviousBodyRotation);
		
		PreviousBodyRotation = BodyRotation;
		YawSpeed = DeltaTime > UE_SMALL_NUMBER ? BodyRotationDelta.Yaw / DeltaTime : 0.f;
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaTime, YawSpeedSmoothLerpSpeed);

		FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
		LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, BodyRotation);

		ForwardSpeed = Velocity.Dot(ControlRotation.Vector());
		RightSpeed = -Velocity.Dot(ControlRotation.Vector().Cross(FVector::UpVector));
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

void UPushAnimInstance::OwnerAimingTagUpdated(const FGameplayTag Tag, int32 Count)
{
	bIsAiming = Count != 0;
}

void UPushAnimInstance::ClearOwnerAimingTagBinding()
{
	if (OwnerAbilitySystemComponent && OwnerAimingTagDelegateHandle.IsValid())
	{
		OwnerAbilitySystemComponent
			->RegisterGameplayTagEvent(PushGameplayTags::Status_Aiming)
			.Remove(OwnerAimingTagDelegateHandle);
	}

	OwnerAbilitySystemComponent = nullptr;
	OwnerAimingTagDelegateHandle.Reset();
}
