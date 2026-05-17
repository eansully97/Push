// Fill out your copyright notice in the Description page of Project Settings.


#include "PushCharacter.h"

APushCharacter::APushCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
