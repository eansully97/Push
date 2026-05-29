
#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "PushGameplayAbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None			UMETA(DisplayName = "None"),
	BasicAttack		UMETA(DisplayName = "BasicAttack"),
	SecondaryAttack	UMETA(DisplayName = "SecondaryAttack"),
	Ability1		UMETA(DisplayName = "Ability1"),
	Ability2		UMETA(DisplayName = "Ability2"),
	Ability3		UMETA(DisplayName = "Ability3"),
	Ability4		UMETA(DisplayName = "Ability4"),
	Confirm			UMETA(DisplayName = "Confirm"),
	Cancel			UMETA(DisplayName = "Cancel"),
};

UENUM(BlueprintType)
enum class EPushGameplayEffectID : uint8
{
	None		UMETA(DisplayName = "None"),
	Death		UMETA(DisplayName = "Death"),
	FullStat	UMETA(DisplayName = "FullStat"),
};

USTRUCT(BlueprintType)
struct FPushGameplayEffect
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	EPushGameplayEffectID EffectID{EPushGameplayEffectID::None};
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> EffectClass{};
};

USTRUCT(BlueprintType)
struct FPushInputActivatedAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> AbilityClass{};

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 Level = 0;
};

USTRUCT(BlueprintType)
struct FPushInputActivatedAbilityDisplayData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EAbilityInputID InputID = EAbilityInputID::None;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayAbility> AbilityClass{};

	UPROPERTY(EditAnywhere)
	int32 Level = 0;
};

USTRUCT(BlueprintType)
struct FGenericDamageEffectDef
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffectClass{};

	UPROPERTY(EditAnywhere)
	FVector PushVelocity = FVector();
};

USTRUCT(BlueprintType)
struct FHeroBaseStats : public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere)
	float Strength;
	
	UPROPERTY(EditAnywhere)
	float StrengthGrowthRate;

	UPROPERTY(EditAnywhere)
	float Intelligence;

	UPROPERTY(EditAnywhere)
	float IntelligenceGrowthRate;

	UPROPERTY(EditAnywhere)
	float BaseMaxHealth;

	UPROPERTY(EditAnywhere)
	float BaseMaxMana;

	UPROPERTY(EditAnywhere)
	float BaseAttackDamage;

	UPROPERTY(EditAnywhere)
	float BaseSpellPower;

	UPROPERTY(EditAnywhere)
	float BaseArmor;

	UPROPERTY(EditAnywhere)
	float BaseSpellResist;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
};
