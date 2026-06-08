// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_ShopItem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UPA_ShopItem;


USTRUCT(BlueprintType)
struct FItemCollection
{
	GENERATED_BODY()
	
public:
	FItemCollection();
	FItemCollection(const TArray<const UPA_ShopItem*>& InItems);
	void AddItem(const UPA_ShopItem* NewItem, bool bAddUnique = false);
	bool Contains(const UPA_ShopItem* Item) const;
	const TArray<const UPA_ShopItem*>& GetItem() const;
	
private:
	UPROPERTY()
	TArray<const UPA_ShopItem*> Items;
};


/**
 * 
 */
UCLASS()
class PUSH_API UPA_ShopItem : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetShopItemAssetType();
	UTexture2D* GetIcon() const;
	FText GetItemName() const{ return ItemName; }
	FText GetItemDescription() const{ return ItemDescription; }
	float GetPrice() const{ return Price; }
	float GetSellPrice() const { return Price / 2; }
	TSubclassOf<UGameplayEffect> GetEquippedEffect() const { return EquippedEffect; }
	TSubclassOf<UGameplayEffect> GetConsumeEffect() const { return ConsumeEffect; }
	TSubclassOf<UGameplayAbility> GetGrantedAbility() const { return GrantedAbility; }
	bool GetIsStackable() const { return bIsStackable; }
	bool GetIsConsumable() const { return bIsConsumable; }
	int32 GetMaxStackCount() const { return MaxStackCount; }
	const TArray<TSoftObjectPtr<UPA_ShopItem>>& GetIngredients() const { return IngredientItems; } 
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	float Price{0.f};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	FText ItemName{FText::GetEmpty()};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	FText ItemDescription{FText::GetEmpty()};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsConsumable{false};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	bool bIsStackable{false};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	int32 MaxStackCount{0};

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TArray<TSoftObjectPtr<UPA_ShopItem>> IngredientItems;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<UGameplayEffect> EquippedEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<UGameplayEffect> ConsumeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ShopItem")
	TSubclassOf<UGameplayAbility> GrantedAbility;
};
