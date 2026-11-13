#pragma once
#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "FalloutFPSCameraManager.generated.h"

UCLASS()
class GASTRO_API AFalloutFPSCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()
public:
    AFalloutFPSCameraManager();
};
