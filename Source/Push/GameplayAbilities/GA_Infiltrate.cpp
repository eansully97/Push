// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Infiltrate.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Push/Player/PushPlayerCharacter.h"

class UAbilityTask_WaitGameplayTagRemoved;

void UGA_Infiltrate::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		StartLaunching();

		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				AbilityMontage
			);

		MontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitStealthEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		StartStealthEventTag,
		nullptr,
		true,
		true
	);

		WaitStealthEvent->EventReceived.AddDynamic(this, &ThisClass::OnStartStealthEvent);
		WaitStealthEvent->ReadyForActivation();
	}
}

void UGA_Infiltrate::StartLaunching()
{
	APushPlayerCharacter* PushCharacter = Cast<APushPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PushCharacter) return;

	FVector InputDirection = PushCharacter->GetMovementInputDirection();

	if (InputDirection.IsNearlyZero())
	{
		InputDirection = PushCharacter->GetActorForwardVector();
	}

	InputDirection.Z = 0.f;
	InputDirection.Normalize();

	const FVector LaunchVelocity =
		(InputDirection + FVector::UpVector).GetSafeNormal() * LaunchSpeed;

	PushSelf(LaunchVelocity);
}

void UGA_Infiltrate::OnStartStealthEvent(FGameplayEventData Payload)
{
	if (StealthEffectClass)
	{
		FActiveGameplayEffectHandle StealthEffectHandle =
			ApplyGameplayEffectToOwner(
				GetCurrentAbilitySpecHandle(),
				GetCurrentActorInfo(),
				GetCurrentActivationInfo(),
				StealthEffectClass.GetDefaultObject(),
				1.f
			);
	}
	UAbilityTask_WaitGameplayTagRemoved* WaitTagRemovedTask =
		UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			StealthTag,
			nullptr
		);

	WaitTagRemovedTask->Removed.AddDynamic(this, &ThisClass::OnStealthRemoved);
	WaitTagRemovedTask->ReadyForActivation();
}

void UGA_Infiltrate::OnStealthRemoved()
{
	K2_EndAbility();
}