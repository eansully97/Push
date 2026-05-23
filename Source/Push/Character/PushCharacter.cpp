// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
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

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AI Perception Stimulus Source Component");

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
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
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

	// Start hidden ALWAYS
	OverheadWidgetComponent->SetHiddenInGame(true);

	if (IsLocallyControlledByPlayer())
	{
		return;
	}

	UOverheadWidget* OverheadWidget =
		Cast<UOverheadWidget>(OverheadWidgetComponent->GetUserWidgetObject());

	if (OverheadWidget)
	{
		OverheadWidget->ConfigureWithASC(GetAbilitySystemComponent());

		GetWorldTimerManager().ClearTimer(OverheadWidgetVisibilityUpdateTimerHandle);

		// Force an immediate update BEFORE timer starts
		UpdateOverheadWidgetVisibility();

		GetWorldTimerManager().SetTimer(
			OverheadWidgetVisibilityUpdateTimerHandle,
			this,
			&ThisClass::UpdateOverheadWidgetVisibility,
			OverheadWidgetVisibilityCheckInterval,
			true);
	}
}

void APushCharacter::UpdateOverheadWidgetVisibility()
{
	if (APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const float DistSq =
			FVector::DistSquared(GetActorLocation(),
								 LocalPlayerPawn->GetActorLocation());

		const bool bShouldShow =
			DistSq <= OverheadWidgetVisibilityRangeSq;

		OverheadWidgetComponent->SetHiddenInGame(!bShouldShow);
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

bool APushCharacter::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UPushAbilitySystemStatics::GetDeadStateTag());
}

void APushCharacter::RespawnImmediately()
{
	if (HasAuthority())
	{
		GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UPushAbilitySystemStatics::GetDeadStateTag()));
	}
}

void APushCharacter::StartDeathSequence()
{
	OnDead();

	if (PushAbilitySystemComponent)
	{
		PushAbilitySystemComponent->CancelAllAbilities();
	}
	
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
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

	SetAIPerceptionStimuliSourceEnabled(true);
	SetStatusGaugeEnabled(true);
	
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	}
	

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
	if (!IsDead())
		return;
	
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
	//Override in child class
}

void APushCharacter::OnRespawn()
{
	//Override in child class
}

void APushCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId APushCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void APushCharacter::OnRep_TeamID()
{
	//Override in child class
}

void APushCharacter::SetAIPerceptionStimuliSourceEnabled(bool bEnabled)
{
	if (!PerceptionStimuliSourceComponent)
		return;

	if (bEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

UAbilitySystemComponent* APushCharacter::GetAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}
