#pragma once

#include "CoreMinimal.h"
#include "Characters/GastroCharacterWithExternalASC.h"
#include "FalloutCharacter.generated.h"

class UCameraComponent;
class UFalloutFPSInputConfig;
struct FInputActionValue;

/** Independent FPS avatar. The PlayerState owns the only ASC. */
UCLASS()
class GASTRO_API AFalloutCharacter : public AGastroCharacterWithExternalASC
{
    GENERATED_BODY()
public:
    AFalloutCharacter();
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void UnPossessed() override;
    virtual bool CanJumpInternal_Implementation() const override;

    UFUNCTION(BlueprintCallable, Category="FPS|Control")
    void Move(const FVector2D& Axis);
    UFUNCTION(BlueprintCallable, Category="FPS|Control")
    void Look(const FVector2D& MouseDelta);
    UFUNCTION(BlueprintCallable, Category="FPS|Control")
    void ResetActionIntent();
    UFUNCTION(BlueprintPure, Category="FPS|Camera")
    float GetVisualEyeHeight() const { return VisualEyeHeight; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|View")
    TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Input")
    TObjectPtr<UFalloutFPSInputConfig> FPSInputConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FPS|Camera", meta=(ClampMin="1", Units="cm"))
    float StandingEyeHeight = 168.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FPS|Camera", meta=(ClampMin="1", Units="cm"))
    float CrouchingEyeHeight = 96.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="FPS|Camera", meta=(ClampMin="0.001", Units="s"))
    float CrouchBlendTime = 0.16f;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
    void MoveInput(const FInputActionValue& Value);
    void LookInput(const FInputActionValue& Value);
    void PlaceCamera();
    float VisualEyeHeight = 168.f;
    TArray<uint32> AxisBindingHandles;
    TWeakObjectPtr<UInputComponent> AxisInputComponent;
};
