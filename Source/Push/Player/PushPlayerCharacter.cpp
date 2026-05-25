// Fill out your copyright notice in the Description page of Project Settings.


#include "PushPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Push/PushGameplayTags.h"
#include "Push/GameplayAbilities/GA_Infiltrate.h"

APushPlayerCharacter::APushPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("View Camera"));
	ViewCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 720.f, 0);
	
}

void APushPlayerCharacter::CacheMaterials()
{
	if (!GetMesh())
	{
		return;
	}

	CachedMaterials = GetMesh()->GetMaterials();
}

void APushPlayerCharacter::SetCachedMaterials()
{
	int32 Index = 0;
	for (const auto& CachedMaterial : CachedMaterials)
	{
		GetMesh()->SetMaterial(Index, CachedMaterial);
		++Index;
	}
}

void APushPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CacheMaterials();
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
	if (ActionValue.Get<bool>())
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed(static_cast<int32>(AbilityInputID));
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased(static_cast<int32>(AbilityInputID));
	}

	if (AbilityInputID == EAbilityInputID::BasicAttack)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, PushGameplayTags::Input_Ability_BasicAttack_Pressed, FGameplayEventData());
		Server_SendGameplayEventToSelf(PushGameplayTags::Input_Ability_BasicAttack_Pressed, FGameplayEventData());
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
