// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "PushPlayerCameraManager.generated.h"

/**
 * Keeps gameplay-triggered camera shakes finite and non-stacking, regardless of
 * whether they were started from C++, PlayerController RPCs, or gameplay cues.
 */
UCLASS()
class PUSH_API APushPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	virtual UCameraShakeBase* StartCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, const FAddCameraShakeParams& Params) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake", meta = (ClampMin = "0.0"))
	float ManagedCameraShakeDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	bool bStopExistingShakeBeforeStarting = true;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shake")
	bool bStopManagedShakeImmediately = true;

private:
	void StopManagedCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass);

	TMap<UClass*, FTimerHandle> ManagedCameraShakeTimers;
};
