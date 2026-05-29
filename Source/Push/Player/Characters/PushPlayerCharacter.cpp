// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Push/Push.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"

APushPlayerCharacter::APushPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	HeroAttributeSet = CreateDefaultSubobject<UPushHeroAttributeSet>("Hero Attribute Set");

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->ProbeChannel = ECC_SpringArm;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("View Camera"));
	ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 720.f, 0);
	
}

void APushPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APushPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APushPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (APlayerController* OwningPlayerController = GetController<APlayerController>())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSubsystem->RemoveMappingContext(GameplayInputMappingContext);
			InputSubsystem->AddMappingContext(GameplayInputMappingContext, 0);
		}
	}
}

bool APushPlayerCharacter::UsesPlayerStateAbilitySystem() const
{
	return true;
}

void APushPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);

		// Optimal Ability Input Setup
		for (const auto& AbilityActionPair : AbilityInputActions)
		{
			EnhancedInputComponent->BindAction(AbilityActionPair.Value, ETriggerEvent::Triggered, this, &ThisClass::HandleAbilityInput, AbilityActionPair.Key);
			EnhancedInputComponent->BindAction(AbilityActionPair.Value, ETriggerEvent::Completed, this, &ThisClass::HandleAbilityInput, AbilityActionPair.Key);
			EnhancedInputComponent->BindAction(AbilityActionPair.Value, ETriggerEvent::Canceled, this, &ThisClass::HandleAbilityInput, AbilityActionPair.Key);
		}
	}
}

void APushPlayerCharacter::HandleLookInput(const FInputActionValue& ActionValue)
{
	const FVector2D InputValue = ActionValue.Get<FVector2D>();

	AddControllerPitchInput(-InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void APushPlayerCharacter::HandleMoveInput(const FInputActionValue& ActionValue)
{
	FVector2D InputValue = ActionValue.Get<FVector2D>();
	InputValue.Normalize();

	AddMovementInput(GetMoveForwardDirection() * InputValue.Y + GetLookRightDirection() * InputValue.X);
}

void APushPlayerCharacter::HandleAbilityInput(const FInputActionValue& ActionValue, EAbilityInputID AbilityInputID)
{
	const bool bIsPressed = ActionValue.Get<bool>();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const bool bIsBasicAttackInput = AbilityInputID == EAbilityInputID::BasicAttack;
	const bool bIsSecondaryAttackInput = AbilityInputID == EAbilityInputID::SecondaryAttack;
	const bool bIsConfirmInput = AbilityInputID == EAbilityInputID::Confirm;
	const bool bIsCancelInput = AbilityInputID == EAbilityInputID::Cancel;
	const bool bGenericConfirmBound = ASC->IsGenericConfirmInputBound(static_cast<int32>(EAbilityInputID::Confirm));
	const bool bGenericCancelBound = ASC->IsGenericCancelInputBound(static_cast<int32>(EAbilityInputID::Cancel));

	if (bIsPressed)
	{
		if (bIsBasicAttackInput && (bGenericConfirmBound || bSuppressBasicAttackUntilRelease))
		{
			bSuppressBasicAttackUntilRelease = true;
			return;
		}

		if (bIsSecondaryAttackInput && (bGenericCancelBound || bSuppressSecondaryAttackUntilRelease))
		{
			bSuppressSecondaryAttackUntilRelease = true;
			return;
		}

		// Confirm/cancel share physical buttons with regular abilities in IMC_Gameplay.
		if (bIsConfirmInput && bGenericConfirmBound)
		{
			bSuppressBasicAttackUntilRelease = true;
		}

		if (bIsCancelInput && bGenericCancelBound)
		{
			bSuppressSecondaryAttackUntilRelease = true;
		}

		ASC->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID));
	}
	else
	{
		if (bIsBasicAttackInput && bSuppressBasicAttackUntilRelease)
		{
			bSuppressBasicAttackUntilRelease = false;
			return;
		}

		if (bIsSecondaryAttackInput && bSuppressSecondaryAttackUntilRelease)
		{
			bSuppressSecondaryAttackUntilRelease = false;
			return;
		}

		ASC->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID));
	}

	if (bIsPressed && bIsBasicAttackInput)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, PushGameplayTags::Input_Ability_BasicAttack_Pressed, FGameplayEventData());
		Server_SendGameplayEventToSelf(PushGameplayTags::Input_Ability_BasicAttack_Pressed, FGameplayEventData());
	}

	if (bIsPressed && bIsSecondaryAttackInput)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, PushGameplayTags::Input_Ability_SecondaryAttack_Pressed, FGameplayEventData());
		Server_SendGameplayEventToSelf(PushGameplayTags::Input_Ability_SecondaryAttack_Pressed, FGameplayEventData());
	}
}

void APushPlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	if (APlayerController* OwningController = GetController<APlayerController>())
	{
		if (bEnabled)
		{
			EnableInput(OwningController);
		}
		else
		{
			DisableInput(OwningController);
		}
	}
}

void APushPlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void APushPlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void APushPlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void APushPlayerCharacter::OnStunRemoved()
{
	if (IsDead())
		return;
	
	SetInputEnabledFromPlayerController(true);
}

void APushPlayerCharacter::OnStealth()
{
	if (!IsLocallyControlledByPlayer())
	{
		SetOverheadWidgetVisibility(true);
	}
}

void APushPlayerCharacter::OnStealthRemoved()
{
	if (!IsLocallyControlledByPlayer())
	{
		UpdateOverheadWidgetVisibility();
	}
}

void APushPlayerCharacter::OnAimStateChanged(bool bIsAiming)
{
	LerpCameraToLocalOffset(bIsAiming ? CameraAimLocalOffset : FVector::ZeroVector);
}

void APushPlayerCharacter::LerpCameraToLocalOffset(const FVector& Goal)
{
	GetWorldTimerManager().ClearTimer(CameraLerpTimerHandle);
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::TickCameraLocalOffset, Goal));
}

void APushPlayerCharacter::TickCameraLocalOffset(FVector Goal)
{
	FVector CurrentLocalOffset = ViewCamera->GetRelativeLocation();
	if (FVector::Dist(CurrentLocalOffset, Goal) < 1.f)
	{
		ViewCamera->SetRelativeLocation(CurrentLocalOffset);
		return;
	}

	float LerpAlpha = FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, Goal, LerpAlpha);
	ViewCamera->SetRelativeLocation(NewLocalOffset);

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ThisClass::TickCameraLocalOffset, Goal));
}

FVector APushPlayerCharacter::GetLookRightDirection() const
{
	return ViewCamera->GetRightVector();
}

FVector APushPlayerCharacter::GetLookForwardDirection() const
{
	return ViewCamera->GetForwardVector();
}

FVector APushPlayerCharacter::GetMoveForwardDirection() const
{
	return FVector::CrossProduct(GetLookRightDirection(), FVector::UpVector);
}

FVector APushPlayerCharacter::GetMovementInputDirection() const
{
	const FVector PendingInput = GetPendingMovementInputVector();

	if (!PendingInput.IsNearlyZero())
	{
		return PendingInput.GetSafeNormal();
	}

	return GetLastMovementInputVector().GetSafeNormal();
}
