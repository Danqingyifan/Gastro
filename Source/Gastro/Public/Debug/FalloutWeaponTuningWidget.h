#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FalloutWeaponTuningWidget.generated.h"

/** Lesson notice; later lessons can add controls without changing the F1 entry. */
UCLASS(meta=(DisableNativeTick))
class GASTRO_API UFalloutWeaponTuningWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
};
