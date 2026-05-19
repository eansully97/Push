// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Push/GAS/PushAbilitySystemComponent.h"
#include "Push/GAS/PushAbilitySystemStatics.h"
#include "Push/GAS/PushAttributeSet.h"
#include "Push/Widgets/OverheadWidget.h"

APushCharacter::APushCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PushAbilitySystemComponent = CreateDefaultSubobject<UPushAbilitySystemComponent>("PushAbilitySystemComponent");
	PushAttributeSet = CreateDefaultSubobject<UPushAttributeSet>("PushAttributeSet");

	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("OverheadWidgetComponent");
	OverheadWidgetComponent->SetupAttachment(GetRootComponent());

	BindChangeDelegates();
}

void APushCharacter::ServerSideInit()
{
	PushAbilitySystemComponent->InitAbilityActorInfo(this,this);
	PushAbilitySystemComponent->ApplyInitialEffects();
	PushAbilitySystemComponent->GiveInitialAbilities();
}

void APushCharacter::ClientSideInit()
{
	PushAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

void APushCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ConfigureOverheadWidget();
	RelativeMeshTransform = GetMesh()->GetRelativeTransform();
}

void APushCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

void APushCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamID)
}

void APushCharacter::BindChangeDelegates()
{
	if (PushAbilitySystemComponent)
	{
		PushAbilitySystemComponent->RegisterGameplayTagEvent(UPushAbilitySystemStatics::GetDeadStateTag()).AddUObject(this, &ThisClass::DeathTagUpdated);
	}
}

void APushCharacter::DeathTagUpdated(FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

bool APushCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalController();
}

void APushCharacter::ConfigureOverheadWidget()
{
	if (!OverheadWidgetComponent)
		return;

	if (IsLocallyControlledByPlayer())
	{
		OverheadWidgetComponent->SetHiddenInGame(true);
		return;
	}
	
	UOverheadWidget* OverheadWidget = Cast<UOverheadWidget>(OverheadWidgetComponent->GetUserWidgetObject());

	if (OverheadWidget)
	{
		OverheadWidget->ConfigureWithASC(GetAbilitySystemComponent());
		OverheadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(OverheadWidgetVisibilityUpdateTimerHandle);
		GetWorldTimerManager().SetTimer(OverheadWidgetVisibilityUpdateTimerHandle, this, &ThisClass::UpdateOverheadWidgetVisibility, OverheadWidgetVisibilityCheckInterval, true);
	}
}

void APushCharacter::UpdateOverheadWidgetVisibility()
{
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LocalPlayerPawn)
	{
		float DistSq = FVector::DistSquared(GetActorLocation(), LocalPlayerPawn->GetActorLocation());
		OverheadWidgetComponent->SetHiddenInGame(DistSq > OverheadWidgetVisibilityRangeSq);

		/*
		 *	Conceptual: Scale Widget with Distance
		 *
		float Distance = FMath::Sqrt(DistSq);
		float Scale = FMath::GetMappedRangeValueClamped(FVector2D(500.f,5000.f), FVector2D(1.f, .4f), Distance);
		OverheadWidgetComponent->SetWorldScale3D(FVector(Scale));
		*/
		
	}
}

void APushCharacter::SetStatusGaugeEnabled(bool bEnabled)
{
	GetWorldTimerManager().ClearTimer(OverheadWidgetVisibilityUpdateTimerHandle);
	if (bEnabled)
	{
		ConfigureOverheadWidget();
	}
	else
	{
		OverheadWidgetComponent->SetHiddenInGame(true);
	}
}

void APushCharacter::StartDeathSequence()
{
	OnDead();
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APushCharacter::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ThisClass::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void APushCharacter::Respawn()
{
	OnRespawn();
	SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

	if (HasAuthority() && GetController())
	{
		TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;
		if (StartSpot.IsValid())
		{
			SetActorTransform(StartSpot->GetActorTransform());
		}
 	}

	if (PushAbilitySystemComponent)
	{
		PushAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void APushCharacter::DeathMontageFinished()
{
	SetRagdollEnabled(true);
}

void APushCharacter::SetRagdollEnabled(bool bEnabled)
{
	if (bEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(RelativeMeshTransform);
	}
}

void APushCharacter::OnDead()
{
	//Override in base class
}

void APushCharacter::OnRespawn()
{
	//Override in base class
}

void APushCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId APushCharacter::GetGenericTeamId() const
{
	return TeamID;
}

UAbilitySystemComponent* APushCharacter::GetAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}
