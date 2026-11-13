#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FalloutFPSHUD.generated.h"

/** A neutral center reticle; ShowDebug FPS3C enables tuning telemetry. */
UCLASS()
class GASTRO_API AFalloutFPSHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
