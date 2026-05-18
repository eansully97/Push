// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Push/GAS/PushAbilitySystemComponent.h"
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
}

void APushCharacter::ServerSideInit()
{
	PushAbilitySystemComponent->InitAbilityActorInfo(this,this);
	PushAbilitySystemComponent->ApplyInitialEffects();
}

void APushCharacter::ClientSideInit()
{
	PushAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

void APushCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ConfigureOverheadWidget();
}

void APushCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
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


UAbilitySystemComponent* APushCharacter::GetAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}