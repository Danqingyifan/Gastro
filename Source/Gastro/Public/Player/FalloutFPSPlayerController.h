#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FalloutFPSPlayerController.generated.h"

class UGastroAbilityInputComponent;
class UEnhancedInputComponent;
class UFalloutFPSInputConfig;
class UInputMappingContext;

UCLASS()
class GASTRO_API AFalloutFPSPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    AFalloutFPSPlayerController();
    void ConfigureFPSInput(UEnhancedInputComponent* Input, const UFalloutFPSInputConfig* Config);
    void ClearFPSInput();
    
    UFUNCTION(BlueprintCallable, Category="FPS|Control")
    void ApplyMouseLook(const FVector2D& MouseDelta);
    
    virtual void FlushPressedKeys() override;
    virtual void SetPawn(APawn* InPawn) override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Input")
    TObjectPtr<UGastroAbilityInputComponent> AbilityInput;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FPS|Input", meta=(ClampMin="0.001"))
    float MouseSensitivity = 1.25f;
protected:
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY(Transient)
    TObjectPtr<UInputMappingContext> ActiveMappingContext;
};
