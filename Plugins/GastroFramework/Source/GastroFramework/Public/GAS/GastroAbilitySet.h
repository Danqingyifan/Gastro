#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GastroAbilitySet.generated.h"

class UGastroAbilitySystemComponent;
class UGastroGameplayAbility;

USTRUCT(BlueprintType)
struct GASTROFRAMEWORK_API FGastroAbilitySetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGastroGameplayAbility> Ability;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 AbilityLevel = 1;

	/** Optional. Leave empty for abilities activated by events or game code. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InputTag;

	bool IsValid() const;
};

/** Per-recipient grant receipt. Keep this on the character/equipment, never on the shared asset. */
USTRUCT(BlueprintType)
struct GASTROFRAMEWORK_API FGastroAbilitySetGrantedHandles
{
	GENERATED_BODY()

	void TakeFromAbilitySystem();
	bool IsEmpty() const { return AbilitySpecHandles.IsEmpty(); }
	const TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() const { return AbilitySpecHandles; }

private:
	friend class UGastroAbilitySet;

	UPROPERTY()
	TWeakObjectPtr<UGastroAbilitySystemComponent> GrantedASC;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
};

/** A reusable server-side ability grant configuration, independent of InputActions and keys. */
UCLASS(BlueprintType, Const)
class GASTROFRAMEWORK_API UGastroAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (TitleProperty = "Ability"))
	TArray<FGastroAbilitySetEntry> GrantedAbilities;

	/** Appends grant handles to a receipt belonging to the same ASC. Does not activate abilities. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS|AbilitySet")
	bool GiveToAbilitySystem(UGastroAbilitySystemComponent* ASC, UPARAM(ref) FGastroAbilitySetGrantedHandles& OutHandles, UObject* SourceObject = nullptr) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS|AbilitySet")
	static void RemoveGrantedAbilities(UPARAM(ref) FGastroAbilitySetGrantedHandles& Handles);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

