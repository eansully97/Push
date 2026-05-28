// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Push/Push.h"
#include "Push/PushGameplayTags.h"
#include "Push/GAS/Components/PushAbilitySystemComponent.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/Player/States/PushPlayerState.h"
#include "Push/Widgets/Overhead/OverheadWidget.h"

APushCharacter::APushCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);
	
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PushAbilitySystemComponent = CreateDefaultSubobject<UPushAbilitySystemComponent>("PushAbilitySystemComponent");
	PushAbilitySystemComponent->SetIsReplicated(true);
	PushAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	PushAttributeSet = CreateDefaultSubobject<UPushAttributeSet>("PushAttributeSet");

	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("OverheadWidgetComponent");
	OverheadWidgetComponent->SetupAttachment(GetRootComponent());

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AI Perception Stimulus Source Component");
}

void APushCharacter::ServerSideInit()
{
	InitializeAbilitySystem();
}

void APushCharacter::ClientSideInit()
{
	InitializeAbilitySystem();
}

void APushCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() && !UsesPlayerStateAbilitySystem())
	{
		ClientSideInit();
	}
	
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

void APushCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (UsesPlayerStateAbilitySystem())
	{
		ClientSideInit();
	}
}

bool APushCharacter::UsesPlayerStateAbilitySystem() const
{
	return false;
}

void APushCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamID)
}

void APushCharacter::InitializeAbilitySystem()
{
	ActiveAbilitySystemComponent = ResolveAbilitySystemComponent();
	ActiveAttributeSet = ResolveAttributeSet();

	if (!ActiveAbilitySystemComponent)
		return;

	if (ActiveAbilitySystemComponent != PushAbilitySystemComponent)
	{
		PushAbilitySystemComponent->SetIsReplicated(false);
		ActiveAbilitySystemComponent->InitializeDefaultsFrom(PushAbilitySystemComponent);
	}

	ActiveAbilitySystemComponent->InitAbilityActorInfo(ActiveAbilitySystemComponent->GetOwner(), this);
	BindChangeDelegates();

	if (HasAuthority())
	{
		ActiveAbilitySystemComponent->ApplyInitialEffects();
		ActiveAbilitySystemComponent->GiveInitialAbilities();
	}

	if (HasActorBegunPlay())
	{
		ConfigureOverheadWidget();
	}
}

UPushAbilitySystemComponent* APushCharacter::ResolveAbilitySystemComponent() const
{
	if (const APushPlayerState* PushPlayerState = GetPlayerState<APushPlayerState>())
	{
		if (UPushAbilitySystemComponent* PlayerStateASC = PushPlayerState->GetPushAbilitySystemComponent())
		{
			return PlayerStateASC;
		}
	}

	return PushAbilitySystemComponent;
}

UPushAttributeSet* APushCharacter::ResolveAttributeSet() const
{
	if (const APushPlayerState* PushPlayerState = GetPlayerState<APushPlayerState>())
	{
		if (UPushAttributeSet* PlayerStateAttributeSet = PushPlayerState->GetPushAttributeSet())
		{
			return PlayerStateAttributeSet;
		}
	}

	return PushAttributeSet;
}

void APushCharacter::BindChangeDelegates()
{
	ClearChangeDelegates();

	if (ActiveAbilitySystemComponent)
	{
		DeadTagDelegateHandle = ActiveAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead)
			.AddUObject(this, &ThisClass::DeathTagUpdated);
		StunTagDelegateHandle = ActiveAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Stun)
			.AddUObject(this, &ThisClass::StunTagUpdated);
		StealthTagDelegateHandle = ActiveAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Stealth)
			.AddUObject(this, &ThisClass::StealthTagUpdated);
		AimingTagDelegateHandle = ActiveAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Aiming)
			.AddUObject(this, &ThisClass::AimingTagUpdated);

		BoundAbilitySystemComponent = ActiveAbilitySystemComponent;
	}
}

void APushCharacter::ClearChangeDelegates()
{
	if (!BoundAbilitySystemComponent)
		return;

	BoundAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Dead).Remove(DeadTagDelegateHandle);
	BoundAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Stun).Remove(StunTagDelegateHandle);
	BoundAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Stealth).Remove(StealthTagDelegateHandle);
	BoundAbilitySystemComponent->RegisterGameplayTagEvent(PushGameplayTags::Status_Aiming).Remove(AimingTagDelegateHandle);

	BoundAbilitySystemComponent = nullptr;
}

void APushCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 Count)
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

void APushCharacter::StunTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (IsDead())
		return;

	if (Count != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnStunRemoved();
		StopAnimMontage(StunMontage);
	}
}

void APushCharacter::StealthTagUpdated(const FGameplayTag Tag, int32 Count)
{
	if (Count != 0)
	{
		OnStealth();
		SetAIPerceptionStimuliSourceEnabled(false);
	}
	else
	{
		OnStealthRemoved();
		if (!IsDead())
		{
			SetAIPerceptionStimuliSourceEnabled(true);
		}
	}
}

void APushCharacter::AimingTagUpdated(const FGameplayTag Tag, int32 Count)
{
	SetIsAiming(Count != 0);
}

void APushCharacter::SetIsAiming(bool bIsAiming)
{
	if (!GetCharacterMovement())
		return;
	
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	OnAimStateChanged(bIsAiming);
}

void APushCharacter::OnAimStateChanged(bool bIsAiming)
{
	// Override in child class
}

bool APushCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalController();
}

void APushCharacter::ConfigureOverheadWidget()
{
	if (GetNetMode() == NM_DedicatedServer)
		return;

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

void APushCharacter::SetOverheadWidgetVisibility(bool Hidden)
{
	if (!OverheadWidgetComponent)
		return;

	OverheadWidgetComponent->SetHiddenInGame(Hidden);
}

void APushCharacter::UpdateOverheadWidgetVisibility()
{
	if (GetNetMode() == NM_DedicatedServer)
		return;

	if (IsLocallyControlledByPlayer())
	{
		SetOverheadWidgetVisibility(true);
		return;
	}

	if (IsInStealth())
		return;
	
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
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC && ASC->HasMatchingGameplayTag(PushGameplayTags::Status_Dead);
}

void APushCharacter::RespawnImmediately()
{
	if (HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			ASC->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(PushGameplayTags::Status_Dead));
		}
	}
}

void APushCharacter::StartDeathSequence()
{
	OnDead();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->CancelAllAbilities();
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
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
	GetWorldTimerManager().ClearTimer(DeathMontageTimerHandle);

	OnRespawn();
	SetRagdollEnabled(false);

	if (!IsInStealth())
	{
		SetAIPerceptionStimuliSourceEnabled(true);
	}
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

	if (ActiveAbilitySystemComponent)
	{
		ActiveAbilitySystemComponent->ApplyFullStatEffect();
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
	USkeletalMeshComponent* MeshComponent = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	if (bEnabled)
	{
		if (MovementComponent)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}

		const FTransform MeshWorldTransform = MeshComponent->GetComponentTransform();
		const FName RagdollCollisionProfileName(TEXT("Ragdoll"));

		MeshComponent->SetCollisionProfileName(RagdollCollisionProfileName);
		MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

		MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		MeshComponent->SetWorldTransform(MeshWorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
		MeshComponent->SetAllBodiesSimulatePhysics(true);
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->WakeAllRigidBodies();
		MeshComponent->bBlendPhysics = true;

		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		MeshComponent->bBlendPhysics = false;
		MeshComponent->SetAllBodiesSimulatePhysics(false);
		MeshComponent->SetSimulatePhysics(false);
		MeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		MeshComponent->SetRelativeTransform(RelativeMeshTransform);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		if (MovementComponent)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
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

void APushCharacter::OnStun()
{
	//Override in child class
}

void APushCharacter::OnStunRemoved()
{
	//Override in child class
}

void APushCharacter::OnStealth()
{
	//Override in child class
}

void APushCharacter::OnStealthRemoved()
{
	//Override in child class
}

bool APushCharacter::IsInStealth() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	return ASC && ASC->HasMatchingGameplayTag(PushGameplayTags::Status_Stealth);
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
	return ResolveAbilitySystemComponent();
}

TArray<FPushInputActivatedAbilityDisplayData> APushCharacter::GetDisplayInputActivatedAbilities() const
{
	if (const UPushAbilitySystemComponent* ASC = ResolveAbilitySystemComponent())
	{
		return ASC->GetDisplayInputActivatedAbilities();
	}

	return {};
}

bool APushCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag,
                                                             const FGameplayEventData& EventData)
{
	return IsAllowedClientGameplayEvent(EventTag, EventData);
}

void APushCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag,
                                                                   const FGameplayEventData& EventData)
{
	if (!IsAllowedClientGameplayEvent(EventTag, EventData))
		return;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

bool APushCharacter::IsAllowedClientGameplayEvent(const FGameplayTag& EventTag, const FGameplayEventData& EventData) const
{
	const bool bIsAllowedInputEvent =
		EventTag == PushGameplayTags::Input_Ability_BasicAttack_Pressed
		|| EventTag == PushGameplayTags::Input_Ability_SecondaryAttack_Pressed;

	return bIsAllowedInputEvent
		&& EventData.TargetData.Num() == 0
		&& EventData.Instigator == nullptr
		&& EventData.Target == nullptr;
}
