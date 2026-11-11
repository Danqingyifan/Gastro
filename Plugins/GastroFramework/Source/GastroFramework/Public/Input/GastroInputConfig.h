#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GastroInputConfig.generated.h"

class UInputAction;

/** Maps a button action to a logical input. Hardware keys remain in the Mapping Context. */
USTRUCT(BlueprintType)
struct GASTROFRAMEWORK_API FGastroAbilityInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag InputTag;

	bool IsValid() const;
};

UCLASS(BlueprintType, Const)
class GASTROFRAMEWORK_API UGastroInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (TitleProperty = "InputTag"))
	TArray<FGastroAbilityInputAction> AbilityInputActions;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};

