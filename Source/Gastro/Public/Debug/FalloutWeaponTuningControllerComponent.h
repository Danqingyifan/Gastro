#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FalloutWeaponTuningControllerComponent.generated.h"

class APlayerController;
class UInputComponent;
class UFalloutWeaponTuningWidget;

UCLASS(ClassGroup=(Debug), meta=(BlueprintSpawnableComponent))
class GASTRO_API UFalloutWeaponTuningControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFalloutWeaponTuningControllerComponent();
    void StartTuning();
    void ShutdownTuning();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void TogglePanel();
    void ClosePanel();

    UPROPERTY(Transient)
    TObjectPtr<APlayerController> Controller;
    UPROPERTY(Transient)
    TObjectPtr<UInputComponent> DebugInputComponent;
    UPROPERTY(Transient)
    TObjectPtr<UFalloutWeaponTuningWidget> Panel;
    bool bStarted = false;
};
