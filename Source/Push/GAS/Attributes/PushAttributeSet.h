// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PushAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class PUSH_API UPushAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, Mana)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, MaxMana)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, AttackDamage)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, SpellPower)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, Armor)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, SpellResist)
	ATTRIBUTE_ACCESSORS(UPushAttributeSet, MoveSpeed)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;

	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;

	UPROPERTY(ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;

	UPROPERTY(ReplicatedUsing = OnRep_AttackDamage)
	FGameplayAttributeData AttackDamage;

	UPROPERTY(ReplicatedUsing = OnRep_SpellPower)
	FGameplayAttributeData SpellPower;

	UPROPERTY(ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;

	UPROPERTY(ReplicatedUsing = OnRep_SpellResist)
	FGameplayAttributeData SpellResist;

	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)
	FGameplayAttributeData MoveSpeed;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& LastValue);
	UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& LastValue);
    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_AttackDamage(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_SpellPower(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_SpellResist(const FGameplayAttributeData& LastValue);
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& LastValue);



};
