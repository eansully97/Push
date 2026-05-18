// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

#include "Components/WidgetComponent.h"
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
	}
}

UAbilitySystemComponent* APushCharacter::GetAbilitySystemComponent() const
{
	return PushAbilitySystemComponent;
}