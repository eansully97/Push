// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Infiltrate.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "Push/Player/Characters/PushPlayerCharacter.h"

UGA_Infiltrate::UGA_Infiltrate()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(PushGameplayTags::Ability_Countess_Infiltrate);
	SetAssetTags(AssetTags);

	BlockAbilitiesWithTag.AddTag(PushGameplayTags::Ability);
	StealthTag = PushGameplayTags::Status_Stealth;
	StartStealthEventTag = PushGameplayTags::GameplayEvent_Ability_Window_Stealth;
}

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

		UAbilityTask_WaitGameplayTagAdded* WaitStealthAddedTask =
			UAbilityTask_WaitGameplayTagAdded::WaitGameplayTagAdd(
				this,
				StealthTag,
				nullptr,
				true
			);

		WaitStealthAddedTask->Added.AddDynamic(this, &ThisClass::OnStealthAdded);
		WaitStealthAddedTask->ReadyForActivation();

		if (!AbilityMontage)
		{
			UE_LOG(LogTemp, Error, TEXT("Infiltrate: AbilityMontage is null."));
			K2_EndAbility();
			return;
		}

		UAbilityTask_PlayMontageAndWait* MontageTask =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this,
				NAME_None,
				AbilityMontage
			);
		MontageTask->OnInterrupted.AddDynamic(this, &UGA_Infiltrate::OnStealthRemoved);
		MontageTask->ReadyForActivation();
	}
}

void UGA_Infiltrate::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	TGuardValue<bool> EndingAbilityGuard(bEndingAbility, true);

	if (!bEndingFromStealthRemoval)
	{
		if (UPushAbilitySystemComponent* PushASC = Cast<UPushAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
		{
			PushASC->AuthBreakStealth();
		}
	}

	bEndingFromStealthRemoval = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Infiltrate::StartLaunching()
{
	if (!K2_HasAuthority())
	{
		return;
	}
	APushPlayerCharacter* PushCharacter = Cast<APushPlayerCharacter>(GetAvatarActorFromActorInfo());
	if (!PushCharacter) return;

	FVector InputDirection = PushCharacter->GetMovementInputDirection();

	if (InputDirection.IsNearlyZero())
	{
		InputDirection = PushCharacter->GetActorForwardVector();
	}

	InputDirection.Z = 0.f;
	InputDirection.Normalize();

	FVector Forward = InputDirection * ForwardLaunchSpeed;
	FVector Up = FVector::UpVector.GetSafeNormal() * UpLaunchSpeed;


	const FVector LaunchVelocity = Forward + Up;
	PushSelf(LaunchVelocity);
}

void UGA_Infiltrate::OnStealthAdded()
{
	SetShouldBlockOtherAbilities(false);

	UAbilityTask_WaitGameplayTagRemoved* WaitTagRemovedTask =
		UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
			this,
			StealthTag,
			nullptr
		);

	WaitTagRemovedTask->Removed.AddDynamic(this, &ThisClass::OnStealthRemoved);
	WaitTagRemovedTask->ReadyForActivation();
}

void UGA_Infiltrate::OnStartStealthEvent(FGameplayEventData Payload)
{
	if (!K2_HasAuthority())
	{
		return;
	}

	if (!StealthEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Infiltrate: StealthEffectClass is null"));
		K2_EndAbility();
		return;
	}

	ApplyGameplayEffectToOwner(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		StealthEffectClass.GetDefaultObject(),
		1.f
	);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("Infiltrate: ASC is null"));
		K2_EndAbility();
		return;
	}

	const bool bHasStealthTag = ASC->HasMatchingGameplayTag(StealthTag);

	if (!bHasStealthTag)
	{
		UE_LOG(LogTemp, Error, TEXT("Infiltrate: GE did not grant expected tag: %s"),
			*StealthTag.ToString());

		K2_EndAbility();
		return;
	}
}

void UGA_Infiltrate::OnStealthRemoved()
{
	if (bEndingAbility)
		return;

	bEndingFromStealthRemoval = true;
	K2_EndAbility();
}
