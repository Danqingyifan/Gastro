#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "EditorSubsystem.h"
#include "FalloutWeaponTuningEditorSubsystem.generated.h"

#if WITH_EDITOR
UCLASS()
class GASTRO_API UFalloutWeaponTuningEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    void OnBeginPIE(bool bIsSimulating);
    void OnEndPIE(bool bIsSimulating);
    bool DiscoverPIEControllers(float DeltaTime);
    void RemovePIEComponents();

    FTSTicker::FDelegateHandle DiscoveryTicker;
};
#endif
