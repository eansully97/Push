// Fill out your copyright notice in the Description page of Project Settings.


#include "PushAbilitySystemComponent.h"

#include "Push/PushGameplayTags.h"
#include "Push/GameplayAbilities/Countess/GA_Infiltrate.h"
#include "Push/GAS/Attributes/PushAttributeSet.h"

UPushAbilitySystemComponent::UPushAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UPushAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HealthUpdated);
	GenericConfirmInputID = static_cast<int32>(EAbilityInputID::Confirm);
	GenericCancelInputID = static_cast<int32>(EAbilityInputID::Cancel);
}

void UPushAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bInitialEffectsApplied)
		return;

	ValidateConfiguredData();
	
	for (const auto& EffectClass : GetInitialEffects())
	{
		AuthApplyGameplayEffect(EffectClass);
	}

	bInitialEffectsApplied = true;
}

void UPushAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (bInitialAbilitiesGranted)
		return;

	ValidateConfiguredData();
	
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

TArray<TSubclassOf<UGameplayEffect>> UPushAbilitySystemComponent::GetInitialEffects() const
{
	TArray<TSubclassOf<UGameplayEffect>> Effects = InitialEffects;
	const TSubclassOf<UGameplayEffect> FullStatEffect = GetFullStatEffect();
	if (FullStatEffect && !Effects.Contains(FullStatEffect))
	{
		Effects.Add(FullStatEffect);
	}

	return Effects;
}

bool UPushAbilitySystemComponent::ValidateConfiguredData() const
{
	bool bIsValid = true;
	TSet<EPushGameplayEffectID> EffectIDs;

	for (const FPushGameplayEffect& GameplayEffect : GameplayEffects)
	{
		if (EffectIDs.Contains(GameplayEffect.EffectID))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has duplicate GameplayEffect entry for id %d."),
				*GetPathName(), static_cast<int32>(GameplayEffect.EffectID));
			bIsValid = false;
		}

		EffectIDs.Add(GameplayEffect.EffectID);

		if (!GameplayEffect.EffectClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null GameplayEffect entry for id %d."),
				*GetPathName(), static_cast<int32>(GameplayEffect.EffectID));
			bIsValid = false;
		}
	}

	if (!GetDeathEffect())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is missing the Death gameplay effect."), *GetPathName());
		bIsValid = false;
	}

	if (!GetFullStatEffect())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s is missing the FullStat gameplay effect."), *GetPathName());
		bIsValid = false;
	}

	for (const auto& AbilityPair : InputActivatedAbilities)
	{
		if (!AbilityPair.Value.AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null input ability for input id %d."),
				*GetPathName(), static_cast<int32>(AbilityPair.Key));
			bIsValid = false;
		}
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s has a null default ability."), *GetPathName());
			bIsValid = false;
		}
	}

	return bIsValid;
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
