#pragma once

#include "CoreMinimal.h"
#include "Input/GastroInputConfig.h"
#include "FalloutFPSInputConfig.generated.h"

class UInputMappingContext;

/** Axes stay native; Boolean actions use Gastro's ability input route. */
UCLASS(BlueprintType, Const)
class GASTRO_API UFalloutFPSInputConfig : public UGastroInputConfig
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Input")
    TObjectPtr<UInputMappingContext> MappingContext;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Input")
    TObjectPtr<UInputAction> MoveAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Input")
    TObjectPtr<UInputAction> MouseLookAction;
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
