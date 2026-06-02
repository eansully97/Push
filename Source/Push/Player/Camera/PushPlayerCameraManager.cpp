// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerCameraManager.h"

#include "Camera/CameraShakeBase.h"
#include "TimerManager.h"

UCameraShakeBase* APushPlayerCameraManager::StartCameraShake(
	TSubclassOf<UCameraShakeBase> ShakeClass,
	const FAddCameraShakeParams& Params)
{
	if (!ShakeClass)
	{
		return nullptr;
	}

	if (bStopExistingShakeBeforeStarting)
	{
		StopAllInstancesOfCameraShake(ShakeClass, true);
	}

	if (FTimerHandle* ExistingTimer = ManagedCameraShakeTimers.Find(ShakeClass.Get()))
	{
		GetWorldTimerManager().ClearTimer(*ExistingTimer);
	}

	UCameraShakeBase* ShakeInstance = Super::StartCameraShake(ShakeClass, Params);

	if (ManagedCameraShakeDuration <= 0.f)
	{
		return ShakeInstance;
	}

	FTimerHandle& TimerHandle = ManagedCameraShakeTimers.FindOrAdd(ShakeClass.Get());
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::StopManagedCameraShake, ShakeClass),
		ManagedCameraShakeDuration,
		false);

	return ShakeInstance;
}

void APushPlayerCameraManager::StopManagedCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (!ShakeClass)
	{
		return;
	}

	StopAllInstancesOfCameraShake(ShakeClass, bStopManagedShakeImmediately);
	ManagedCameraShakeTimers.Remove(ShakeClass.Get());
}
