// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemComponent.h"

#include "Push/PushGameplayTags.h"
#include "Push/GameplayAbilities/Countess/GA_Infiltrate.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"

UPushAbilitySystemComponent::UPushAbilitySystemComponent()
{
	GenericConfirmInputID = static_cast<int32>(EAbilityInputID::Confirm);
	GenericCancelInputID = static_cast<int32>(EAbilityInputID::Cancel);
}

bool UPushAbilitySystemComponent::InitializeBaseAttributes()
{
	if (!GetOwner())
		return false;

	const AActor* StatsActor = GetAvatarActor() ? GetAvatarActor() : GetOwner();

	if (!BaseStatsData)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no BaseStatsData configured; base attributes were not initialized for %s."),
			*GetValidationContext(),
			*GetNameSafe(StatsActor));
		return false;
	}

	const FHeroBaseStats* BaseStats = FindBaseStatsForActor(StatsActor);
	if (!BaseStats)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s could not find a BaseStatsData row for %s (%s)."),
			*GetValidationContext(),
			*GetNameSafe(StatsActor),
			*GetNameSafe(StatsActor->GetClass()));
		return false;
	}

	SetNumericAttributeBase(UPushAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
	SetNumericAttributeBase(UPushAttributeSet::GetMaxManaAttribute(), BaseStats->BaseMaxMana);
	SetNumericAttributeBase(UPushAttributeSet::GetAttackDamageAttribute(), BaseStats->BaseAttackDamage);
	SetNumericAttributeBase(UPushAttributeSet::GetSpellPowerAttribute(), BaseStats->BaseSpellPower);
	SetNumericAttributeBase(UPushAttributeSet::GetArmorAttribute(), BaseStats->BaseArmor);
	SetNumericAttributeBase(UPushAttributeSet::GetSpellResistAttribute(), BaseStats->BaseSpellResist);
	SetNumericAttributeBase(UPushAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);

	if (HasAttributeSetForAttribute(UPushHeroAttributeSet::GetStrengthAttribute()))
	{
		SetNumericAttributeBase(UPushHeroAttributeSet::GetStrengthAttribute(), BaseStats->Strength);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetIntelligenceAttribute(), BaseStats->Intelligence);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetStrengthGrowthRateAttribute(), BaseStats->StrengthGrowthRate);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetIntelligenceGrowthRateAttribute(), BaseStats->IntelligenceGrowthRate);
	}

	return true;
}

void UPushAbilitySystemComponent::ServerSideInit(bool bApplyInitialEffects)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	BindHealthAttributeDelegate();

	const bool bShouldApplyInitialEffects = bApplyInitialEffects && HasInitialEffects();
	ValidateConfiguredDataOnce(bApplyInitialEffects);

	const bool bBaseAttributesInitialized = bShouldApplyInitialEffects ? false : InitializeBaseAttributes();

	ApplyStartupEffects(bBaseAttributesInitialized, bShouldApplyInitialEffects);
	GiveInitialAbilities();
}

void UPushAbilitySystemComponent::ApplyStartupEffects(bool bBaseAttributesInitialized, bool bApplyInitialEffects)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bStartupEffectsApplied)
		return;

	const bool bInitialEffectsApplied = bApplyInitialEffects && ApplyInitialEffects();
	if (bBaseAttributesInitialized || bInitialEffectsApplied)
	{
		AuthApplyGameplayEffect(GetFullStatEffect());
	}

	bStartupEffectsApplied = true;
}

bool UPushAbilitySystemComponent::HasInitialEffects() const
{
	return InitialEffects.ContainsByPredicate(
		[](const TSubclassOf<UGameplayEffect>& EffectClass)
		{
			return EffectClass != nullptr;
		});
}

bool UPushAbilitySystemComponent::ApplyInitialEffects()
{
	bool bAppliedAnyEffect = false;
	const TSubclassOf<UGameplayEffect> FullStatEffect = GetFullStatEffect();

	for (const TSubclassOf<UGameplayEffect>& EffectClass : InitialEffects)
	{
		if (!EffectClass || EffectClass == FullStatEffect)
		{
			continue;
		}

		AuthApplyGameplayEffect(EffectClass);
		bAppliedAnyEffect = true;
	}

	return bAppliedAnyEffect;
}

void UPushAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bInitialAbilitiesGranted)
		return;

	for (const auto& AbilityPair : InputActivatedAbilities)
	{
		const FPushInputActivatedAbility& Ability = AbilityPair.Value;
		if (Ability.AbilityClass)
		{
			GiveAbility(FGameplayAbilitySpec(Ability.AbilityClass, Ability.Level, static_cast<int32>(AbilityPair.Key), nullptr));
		}
	}

	for (const auto& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, nullptr));
		}
	}

	bInitialAbilitiesGranted = true;
}

void UPushAbilitySystemComponent::BindHealthAttributeDelegate()
{
	if (bHealthAttributeDelegateBound)
	{
		return;
	}

	HealthAttributeChangedDelegateHandle =
		GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ThisClass::HealthUpdated);
	bHealthAttributeDelegateBound = true;
}

void UPushAbilitySystemComponent::ApplyFullStatEffect()
{
	AuthApplyGameplayEffect(GetFullStatEffect());
}

void UPushAbilitySystemComponent::RemoveTransientEffectsForDeath()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	const TArray<FActiveGameplayEffectHandle> ActiveEffectHandles = GetActiveEffects(FGameplayEffectQuery());
	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : ActiveEffectHandles)
	{
		const FActiveGameplayEffect* ActiveEffect = GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect || ShouldPersistActiveEffectThroughDeath(*ActiveEffect))
		{
			continue;
		}

		RemoveActiveGameplayEffect(ActiveEffectHandle);
	}
}

void UPushAbilitySystemComponent::AuthBreakStealth()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (!HasMatchingGameplayTag(PushGameplayTags::Status_Stealth))
		return;

	RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(PushGameplayTags::Status_Stealth));
}

void UPushAbilitySystemComponent::InitializeDefaultsFrom(const UPushAbilitySystemComponent* DefaultsSource)
{
	if (!DefaultsSource || DefaultsSource == this)
		return;

	GameplayEffects = DefaultsSource->GameplayEffects;
	InitialEffects = DefaultsSource->InitialEffects;
	InputActivatedAbilities = DefaultsSource->InputActivatedAbilities;
	DefaultAbilities = DefaultsSource->DefaultAbilities;
	BaseStatsData = DefaultsSource->BaseStatsData;
}

const TMap<EAbilityInputID, FPushInputActivatedAbility>& UPushAbilitySystemComponent::GetInputActivatedAbilities() const
{
	return InputActivatedAbilities;
}

TArray<FPushInputActivatedAbilityDisplayData> UPushAbilitySystemComponent::GetDisplayInputActivatedAbilities() const
{
	TArray<FPushInputActivatedAbilityDisplayData> DisplayAbilities;
	for (const auto& AbilityPair : InputActivatedAbilities)
	{
		if (AbilityPair.Key == EAbilityInputID::None
			|| AbilityPair.Key == EAbilityInputID::BasicAttack
			|| AbilityPair.Key == EAbilityInputID::Confirm
			|| AbilityPair.Key == EAbilityInputID::Cancel
			|| !AbilityPair.Value.AbilityClass)
		{
			continue;
		}

		FPushInputActivatedAbilityDisplayData DisplayData;
		DisplayData.InputID = AbilityPair.Key;
		DisplayData.AbilityClass = AbilityPair.Value.AbilityClass;
		DisplayData.Level = AbilityPair.Value.Level;
		DisplayAbilities.Add(DisplayData);
	}

	DisplayAbilities.Sort(
		[](const FPushInputActivatedAbilityDisplayData& Left, const FPushInputActivatedAbilityDisplayData& Right)
		{
			return static_cast<uint8>(Left.InputID) < static_cast<uint8>(Right.InputID);
		});

	return DisplayAbilities;
}

TSubclassOf<UGameplayEffect> UPushAbilitySystemComponent::GetGameplayEffect(
	EPushGameplayEffectID GameplayEffectID) const
{
	const FPushGameplayEffect* GameplayEffect = GameplayEffects.FindByPredicate(
		[GameplayEffectID](const FPushGameplayEffect& Effect)
		{
			return Effect.EffectID == GameplayEffectID;
		});

	if (GameplayEffect)
	{
		return GameplayEffect->EffectClass;
	}

	return nullptr;
}

TSubclassOf<UGameplayEffect> UPushAbilitySystemComponent::GetDeathEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::Death);
}

TSubclassOf<UGameplayEffect> UPushAbilitySystemComponent::GetFullStatEffect() const
{
	return GetGameplayEffect(EPushGameplayEffectID::FullStat);
}

bool UPushAbilitySystemComponent::ValidateConfiguredData() const
{
	bool bIsValid = true;
	TSet<EPushGameplayEffectID> EffectIDs;
	bool bHasConfiguredAbility = false;
	const FString ValidationContext = GetValidationContext();

	for (const FPushGameplayEffect& GameplayEffect : GameplayEffects)
	{
		if (EffectIDs.Contains(GameplayEffect.EffectID))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has duplicate GameplayEffect entry for id %d."),
				*ValidationContext, static_cast<int32>(GameplayEffect.EffectID));
			bIsValid = false;
		}

		EffectIDs.Add(GameplayEffect.EffectID);

		if (!GameplayEffect.EffectClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null GameplayEffect entry for id %d."),
				*ValidationContext, static_cast<int32>(GameplayEffect.EffectID));
			bIsValid = false;
		}
	}

	if (!GetDeathEffect())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is missing the Death gameplay effect."), *ValidationContext);
		bIsValid = false;
	}

	if (!GetFullStatEffect())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is missing the FullStat gameplay effect."), *ValidationContext);
		bIsValid = false;
	}

	for (const auto& AbilityPair : InputActivatedAbilities)
	{
		if (AbilityPair.Key == EAbilityInputID::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has an input-activated ability assigned to input id None. Move non-input startup abilities to DefaultAbilities, or assign a real input id."),
				*ValidationContext);
			bIsValid = false;
		}

		if (!AbilityPair.Value.AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null input ability for input id %d."),
				*ValidationContext, static_cast<int32>(AbilityPair.Key));
			bIsValid = false;
		}
		else
		{
			bHasConfiguredAbility = true;
		}
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null default ability."), *ValidationContext);
			bIsValid = false;
		}
		else
		{
			bHasConfiguredAbility = true;
		}
	}

	if (!bHasConfiguredAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no configured input or default startup abilities."), *ValidationContext);
		bIsValid = false;
	}

	return bIsValid;
}

bool UPushAbilitySystemComponent::ValidateConfiguredDataOnce(bool bApplyInitialEffects)
{
	if (bConfiguredDataValidated)
	{
		return bConfiguredDataValid;
	}

	bConfiguredDataValidated = true;
	const bool bConfiguredDataValidNow = ValidateConfiguredData();
	const bool bStartupConfigurationValid = ValidateStartupConfiguration(bApplyInitialEffects);
	bConfiguredDataValid = bConfiguredDataValidNow && bStartupConfigurationValid;
	return bConfiguredDataValid;
}

bool UPushAbilitySystemComponent::ValidateStartupConfiguration(bool bApplyInitialEffects) const
{
	const bool bHasValidInitialEffect = bApplyInitialEffects && HasInitialEffects();
	const bool bBaseAttributesExpected = !bHasValidInitialEffect;
	const AActor* StatsActor = GetAvatarActor() ? GetAvatarActor() : GetOwner();

	return ValidateBaseStatsConfiguration(bBaseAttributesExpected, StatsActor);
}

bool UPushAbilitySystemComponent::ValidateBaseStatsConfiguration(bool bBaseAttributesExpected, const AActor* StatsActor) const
{
	if (!bBaseAttributesExpected)
	{
		return true;
	}

	const FString ValidationContext = GetValidationContext();
	if (!BaseStatsData)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s expects BaseStatsData for startup attributes, but none is configured."),
			*ValidationContext);
		return false;
	}

	if (!StatsActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s expects BaseStatsData for startup attributes, but has no owner or avatar actor."),
			*ValidationContext);
		return false;
	}

	if (!FindBaseStatsForActor(StatsActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s expects BaseStatsData for startup attributes, but no row matches %s (%s)."),
			*ValidationContext,
			*GetNameSafe(StatsActor),
			*GetNameSafe(StatsActor->GetClass()));
		return false;
	}

	return true;
}

const FHeroBaseStats* UPushAbilitySystemComponent::FindBaseStatsForActor(const AActor* StatsActor) const
{
	if (!BaseStatsData || !StatsActor)
	{
		return nullptr;
	}

	for (const auto& DataPair : BaseStatsData->GetRowMap())
	{
		const FHeroBaseStats* BaseStats = BaseStatsData->FindRow<FHeroBaseStats>(DataPair.Key, "");
		if (BaseStats && BaseStats->Class && StatsActor->IsA(BaseStats->Class))
		{
			return BaseStats;
		}
	}

	return nullptr;
}

FString UPushAbilitySystemComponent::GetValidationContext() const
{
	const AActor* OwningActor = GetOwner();
	const AActor* AvatarActorForContext = GetAvatarActor();
	return FString::Printf(TEXT("%s (Owner=%s, Avatar=%s, AvatarClass=%s)"),
		*GetPathName(),
		*GetNameSafe(OwningActor),
		*GetNameSafe(AvatarActorForContext),
		AvatarActorForContext ? *GetNameSafe(AvatarActorForContext->GetClass()) : TEXT("None"));
}

void UPushAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (ShouldAbilityActivationBreakStealth(Handle, Ability))
	{
		AuthBreakStealth();
	}
}

void UPushAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner()) return;

	if (ChangeData.NewValue <= 0 && GetOwner()->HasAuthority() && !HasMatchingGameplayTag(PushGameplayTags::Status_Dead))
	{
		RemoveTransientEffectsForDeath();
		AuthApplyGameplayEffect(GetDeathEffect());
	}
}

void UPushAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level)
{
	if (!GameplayEffect || !GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
	if (!EffectSpecHandle.IsValid() || !EffectSpecHandle.Data.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s failed to create gameplay effect spec for %s."),
			*GetPathName(),
			*GetNameSafe(GameplayEffect.Get()));
		return;
	}

	ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}

bool UPushAbilitySystemComponent::ShouldAbilityActivationBreakStealth(
	const FGameplayAbilitySpecHandle Handle,
	const UGameplayAbility* Ability) const
{
	if (!Ability || Ability->IsA(UGA_Infiltrate::StaticClass()))
		return false;

	const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(Handle);
	if (!AbilitySpec)
		return false;

	const int32 BasicAttackInputID = static_cast<int32>(EAbilityInputID::BasicAttack);
	const int32 Ability4InputID = static_cast<int32>(EAbilityInputID::Ability4);

	return AbilitySpec->InputID >= BasicAttackInputID
		&& AbilitySpec->InputID <= Ability4InputID;
}

bool UPushAbilitySystemComponent::ShouldPersistActiveEffectThroughDeath(const FActiveGameplayEffect& ActiveEffect) const
{
	FGameplayTagContainer GrantedTags;
	ActiveEffect.Spec.GetAllGrantedTags(GrantedTags);

	if (GrantedTags.HasTag(PushGameplayTags::Cooldown) || GrantedTags.HasTagExact(PushGameplayTags::Status_Dead))
	{
		return true;
	}

	const TSubclassOf<UGameplayEffect> DeathEffectClass = GetDeathEffect();
	return DeathEffectClass
		&& ActiveEffect.Spec.Def
		&& ActiveEffect.Spec.Def->GetClass()->IsChildOf(DeathEffectClass);
}
