// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Push/PushGameplayTags.h"
#include "Push/GameplayAbilities/Countess/GA_Infiltrate.h"
#include "Push/GAS/PushAbilitySystemStatics.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"
#include "Push/GAS/Attributes/PushHeroAttributeSet.h"
#include "Push/GAS/Data/PA_AbilitySystemGenerics.h"

namespace
{
	bool IsUpgradeableAbilityInputID(EAbilityInputID InputID)
	{
		switch (InputID)
		{
		case EAbilityInputID::Ability1:
		case EAbilityInputID::Ability2:
		case EAbilityInputID::Ability3:
		case EAbilityInputID::Ability4:
		case EAbilityInputID::SecondaryAttack:
			return true;
		default:
			return false;
		}
	}
}

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

	if (!AbilitySystemGenerics || !AbilitySystemGenerics->GetBaseDataTable())
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

	if (HasHeroAttributes())
	{
		SetNumericAttributeBase(UPushHeroAttributeSet::GetStrengthAttribute(), BaseStats->Strength);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetIntelligenceAttribute(), BaseStats->Intelligence);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetStrengthGrowthRateAttribute(), BaseStats->StrengthGrowthRate);
		SetNumericAttributeBase(UPushHeroAttributeSet::GetIntelligenceGrowthRateAttribute(), BaseStats->IntelligenceGrowthRate);

		const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();
		if (ExperienceCurve && ExperienceCurve->GetNumKeys() > 0)
		{
			int32 MaxLevel = ExperienceCurve->GetNumKeys();
			SetNumericAttributeBase(UPushHeroAttributeSet::GetMaxLevelAttribute(), MaxLevel);
		
			float MaxEXP = ExperienceCurve->GetKeyValue(ExperienceCurve->GetLastKeyHandle());
			SetNumericAttributeBase(UPushHeroAttributeSet::GetMaxLevelExperienceAttribute(), MaxEXP);
		}

		ExperienceUpdated(FOnAttributeChangeData());
	}

	return true;
}

void UPushAbilitySystemComponent::ServerSideInit()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	BindHealthAttributeDelegate();
	BindManaAttributeDelegate();
	if (HasHeroAttributes())
	{
		BindExperienceAttributeDelegate();
	}

	ValidateConfiguredDataOnce();

	const bool bConfiguredStartupEffectsApplied = HasStartupEffects() && ApplyConfiguredStartupEffects();
	const bool bBaseAttributesInitialized = bConfiguredStartupEffectsApplied ? false : InitializeBaseAttributes();

	ApplyPostStartupEffects(bConfiguredStartupEffectsApplied || bBaseAttributesInitialized);
	GiveInitialAbilities();
}

void UPushAbilitySystemComponent::ApplyPostStartupEffects(bool bStartupAttributesInitialized)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bStartupEffectsApplied)
		return;

	if (!AbilitySystemGenerics)
		return;

	if (bStartupAttributesInitialized)
	{
		if (HasHeroAttributes())
		{
			AuthApplyGameplayEffect(AbilitySystemGenerics->GetHealthRegenEffect());
			AuthApplyGameplayEffect(AbilitySystemGenerics->GetManaRegenEffect());
			AuthApplyGameplayEffect(AbilitySystemGenerics->GetAddHeroTagEffect());
			AuthApplyGameplayEffect(AbilitySystemGenerics->GetLevelStatsEffect());
		}
		AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
	}

	bStartupEffectsApplied = true;
}

bool UPushAbilitySystemComponent::HasStartupEffects() const
{
	const TSubclassOf<UGameplayEffect> FullStatEffect = AbilitySystemGenerics
		? AbilitySystemGenerics->GetFullStatEffect()
		: nullptr;

	return StartupEffects.ContainsByPredicate(
		[FullStatEffect](const TSubclassOf<UGameplayEffect>& EffectClass)
		{
			return EffectClass != nullptr && EffectClass != FullStatEffect;
		});
}

bool UPushAbilitySystemComponent::ApplyConfiguredStartupEffects()
{
	bool bAppliedAnyEffect = false;
	const TSubclassOf<UGameplayEffect> FullStatEffect = AbilitySystemGenerics
		? AbilitySystemGenerics->GetFullStatEffect()
		: nullptr;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : StartupEffects)
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

bool UPushAbilitySystemComponent::HasHeroAttributes() const
{
	return HasAttributeSetForAttribute(UPushHeroAttributeSet::GetStrengthAttribute());
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

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : PassiveAbilities)
	{
		if (AbilityClass)
		{
			GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, nullptr));
		}
	}

	if (!AbilitySystemGenerics)
	{
		bInitialAbilitiesGranted = true;
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitySystemGenerics->GetDefaultAbilities())
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

void UPushAbilitySystemComponent::BindManaAttributeDelegate()
{
	if (bManaAttributeDelegateBound)
	{
		return;
	}

	ManaAttributeChangedDelegateHandle =
		GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetManaAttribute())
		.AddUObject(this, &ThisClass::ManaUpdated);
	bManaAttributeDelegateBound = true;
}

void UPushAbilitySystemComponent::BindExperienceAttributeDelegate()
{
	if (bExperienceAttributeDelegateBound)
	{
		return;
	}

	ExperienceAttributeChangedDelegateHandle =
		GetGameplayAttributeValueChangeDelegate(UPushHeroAttributeSet::GetExperienceAttribute())
		.AddUObject(this, &ThisClass::ExperienceUpdated);
	bExperienceAttributeDelegateBound = true;
}

void UPushAbilitySystemComponent::ApplyFullStatEffect()
{
	if (AbilitySystemGenerics)
	{
		AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
	}
}

void UPushAbilitySystemComponent::AuthApplyDeathStatusEffect()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !AbilitySystemGenerics)
		return;

	AuthApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());
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

	InputActivatedAbilities = DefaultsSource->InputActivatedAbilities;
	PassiveAbilities = DefaultsSource->PassiveAbilities;
	StartupEffects = DefaultsSource->StartupEffects;
	AbilitySystemGenerics = DefaultsSource->AbilitySystemGenerics;
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

bool UPushAbilitySystemComponent::IsAtMaxLevel() const
{
	bool bFoundCurrentLevel = false;
	float CurrentLevel = GetGameplayAttributeValue(UPushHeroAttributeSet::GetLevelAttribute(), bFoundCurrentLevel);

	bool bFoundMaxLevel = false;
	float MaxLevel = GetGameplayAttributeValue(UPushHeroAttributeSet::GetMaxLevelAttribute(), bFoundMaxLevel);

	return bFoundCurrentLevel && bFoundMaxLevel && MaxLevel > 0.f && CurrentLevel >= MaxLevel;
}

bool UPushAbilitySystemComponent::ValidateConfiguredData() const
{
	bool bIsValid = true;
	TSet<EPushGameplayEffectID> EffectIDs;
	bool bHasConfiguredAbility = false;
	const FString ValidationContext = GetValidationContext();

	if (!AbilitySystemGenerics)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no AbilitySystemGenerics data asset configured."), *ValidationContext);
		bIsValid = false;
	}

	if (AbilitySystemGenerics)
	{
		for (const FPushGameplayEffect& GameplayEffect : AbilitySystemGenerics->GetGameplayEffects())
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

		if (!AbilitySystemGenerics->GetDeathEffect())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is missing the Death gameplay effect."), *ValidationContext);
			bIsValid = false;
		}

		if (!AbilitySystemGenerics->GetFullStatEffect())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is missing the FullStat gameplay effect."), *ValidationContext);
			bIsValid = false;
		}

		if (HasHeroAttributes() && !AbilitySystemGenerics->GetLevelStatsEffect())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is missing the LevelStats gameplay effect."), *ValidationContext);
			bIsValid = false;
		}
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

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : PassiveAbilities)
	{
		if (!AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null passive ability."), *ValidationContext);
			bIsValid = false;
		}
		else
		{
			bHasConfiguredAbility = true;
		}
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : StartupEffects)
	{
		if (!EffectClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null startup effect."), *ValidationContext);
			bIsValid = false;
		}
	}

	if (AbilitySystemGenerics)
	{
		for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilitySystemGenerics->GetDefaultAbilities())
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
	}

	if (!bHasConfiguredAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no configured input or default startup abilities."), *ValidationContext);
		bIsValid = false;
	}

	return bIsValid;
}

void UPushAbilitySystemComponent::Server_UpgradeAbilityWithID_Implementation(EAbilityInputID InputID)
{
	if (!IsUpgradeableAbilityInputID(InputID))
		return;

	bool bFound = false;
	float UpgradePoint = GetGameplayAttributeValue(UPushHeroAttributeSet::GetUpgradePointAttribute(), bFound);
	if (!bFound || UpgradePoint <= 0.0f)
		return;

	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID(static_cast<int32>(InputID));
	if (!AbilitySpec || UPushAbilitySystemStatics::IsAbilityMaxLevel(*AbilitySpec))
		return;

	SetNumericAttributeBase(UPushHeroAttributeSet::GetUpgradePointAttribute(), UpgradePoint - 1);
	AbilitySpec->Level += 1;
	MarkAbilitySpecDirty(*AbilitySpec);
}

bool UPushAbilitySystemComponent::Server_UpgradeAbilityWithID_Validate(EAbilityInputID InputID)
{
	return true;
}

bool UPushAbilitySystemComponent::ValidateConfiguredDataOnce()
{
	if (bConfiguredDataValidated)
	{
		return bConfiguredDataValid;
	}

	bConfiguredDataValidated = true;
	const bool bConfiguredDataValidNow = ValidateConfiguredData();
	const bool bStartupConfigurationValid = ValidateStartupConfiguration();
	bConfiguredDataValid = bConfiguredDataValidNow && bStartupConfigurationValid;
	return bConfiguredDataValid;
}

bool UPushAbilitySystemComponent::ValidateStartupConfiguration() const
{
	const bool bBaseAttributesExpected = !HasStartupEffects();
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
	if (!AbilitySystemGenerics)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s expects AbilitySystemGenerics for startup attributes, but none is configured."),
			*ValidationContext);
		return false;
	}

	if (!AbilitySystemGenerics->GetBaseDataTable())
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
	if (!AbilitySystemGenerics || !AbilitySystemGenerics->GetBaseDataTable() || !StatsActor)
	{
		return nullptr;
	}

	const UDataTable* BaseStatsData = AbilitySystemGenerics->GetBaseDataTable();
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

void UPushAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.Ability)
		{
			AbilitySpecDirtiedCallbacks.Broadcast(AbilitySpec);
		}
	}
}

void UPushAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UPushAttributeSet::GetMaxHealthAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		if (!HasMatchingGameplayTag(PushGameplayTags::Status_Health_Full))
		{
			AddLooseGameplayTag(PushGameplayTags::Status_Health_Full);
		}
	}
	else
	{
		RemoveLooseGameplayTag(PushGameplayTags::Status_Health_Full);
	}

	if (ChangeData.NewValue <= 0 && !HasMatchingGameplayTag(PushGameplayTags::Status_Dead))
	{
		if (!HasMatchingGameplayTag(PushGameplayTags::Status_Health_Empty))
		{
			AddLooseGameplayTag(PushGameplayTags::Status_Health_Empty);
			const TSubclassOf<UGameplayEffect> DeathEffect = AbilitySystemGenerics
				? AbilitySystemGenerics->GetDeathEffect()
				: nullptr;
			if (DeathEffect)
			{
				RemoveTransientEffectsForDeath();
				AuthApplyGameplayEffect(DeathEffect);

				FGameplayEventData DeadAbilityEventData;
				if (ChangeData.GEModData)
					DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), PushGameplayTags::Status_Dead, DeadAbilityEventData);
			}
		}
		else
		{
			RemoveLooseGameplayTag(PushGameplayTags::Status_Health_Empty);
		}
	}
}

void UPushAbilitySystemComponent::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxMana = GetGameplayAttributeValue(UPushAttributeSet::GetMaxManaAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxMana)
	{
		if (!HasMatchingGameplayTag(PushGameplayTags::Status_Mana_Full))
		{
			AddLooseGameplayTag(PushGameplayTags::Status_Mana_Full);
		}
	}
	else
	{
		RemoveLooseGameplayTag(PushGameplayTags::Status_Mana_Full);
	}

	if (ChangeData.NewValue <= 0 && !HasMatchingGameplayTag(PushGameplayTags::Status_Dead))
	{
		if (!HasMatchingGameplayTag(PushGameplayTags::Status_Mana_Empty))
		{
			AddLooseGameplayTag(PushGameplayTags::Status_Mana_Empty);
		}
		else
		{
			RemoveLooseGameplayTag(PushGameplayTags::Status_Mana_Empty);
		}
	}
}

void UPushAbilitySystemComponent::ExperienceUpdated(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (!HasHeroAttributes())
		return;

	if (IsAtMaxLevel())
		return;

	if (!AbilitySystemGenerics)
		return;

	float CurrentExp = ChangeData.NewValue;
	const FRealCurve* ExperienceCurve = AbilitySystemGenerics->GetExperienceCurve();
	if (!ExperienceCurve)
	{
		UE_LOG(LogTemp, Warning, TEXT("NO XP DATA"))
		return;
	}

	float PrevLevelExp = 0.f;
	float NextLevelExp = 0.f;
	float NewLevel = 1.f;

	for (auto Iter = ExperienceCurve->GetKeyHandleIterator(); Iter; ++Iter)
	{
		float ExperienceToReachLevel = ExperienceCurve->GetKeyValue(*Iter);
		if (CurrentExp < ExperienceToReachLevel)
		{
			NextLevelExp = ExperienceToReachLevel;
			break;
		}

		PrevLevelExp = ExperienceToReachLevel;
		NewLevel = Iter.GetIndex() + 1;
	}

	float CurrentLevel = GetNumericAttributeBase(UPushHeroAttributeSet::GetLevelAttribute());
	float CurrentUpgradePoint = GetNumericAttributeBase(UPushHeroAttributeSet::GetUpgradePointAttribute());

	float LevelUpgraded = NewLevel - CurrentLevel;
	float NewUpgradePoint = CurrentUpgradePoint + LevelUpgraded;

	SetNumericAttributeBase(UPushHeroAttributeSet::GetLevelAttribute(), NewLevel);
	SetNumericAttributeBase(UPushHeroAttributeSet::GetPrevLevelExperienceAttribute(), PrevLevelExp);
	SetNumericAttributeBase(UPushHeroAttributeSet::GetNextLevelExperienceAttribute(), NextLevelExp);
	SetNumericAttributeBase(UPushHeroAttributeSet::GetUpgradePointAttribute(), NewUpgradePoint);
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

	const TSubclassOf<UGameplayEffect> DeathEffectClass = AbilitySystemGenerics
		? AbilitySystemGenerics->GetDeathEffect()
		: nullptr;
	return DeathEffectClass
		&& ActiveEffect.Spec.Def
		&& ActiveEffect.Spec.Def->GetClass()->IsChildOf(DeathEffectClass);
}
