#include "Debug/FalloutWeaponTuningEditorSubsystem.h"

#if WITH_EDITOR

#include "Debug/FalloutWeaponTuningControllerComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Containers/Ticker.h"

void UFalloutWeaponTuningEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FEditorDelegates::BeginPIE.AddUObject(this, &ThisClass::OnBeginPIE);
    FEditorDelegates::EndPIE.AddUObject(this, &ThisClass::OnEndPIE);
}

void UFalloutWeaponTuningEditorSubsystem::Deinitialize()
{
    if (DiscoveryTicker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(DiscoveryTicker);
    RemovePIEComponents();
    FEditorDelegates::BeginPIE.RemoveAll(this);
    FEditorDelegates::EndPIE.RemoveAll(this);
    Super::Deinitialize();
}

void UFalloutWeaponTuningEditorSubsystem::OnBeginPIE(bool bIsSimulating)
{
    if (DiscoveryTicker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(DiscoveryTicker);
    DiscoveryTicker = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &ThisClass::DiscoverPIEControllers), 0.1f);
}

void UFalloutWeaponTuningEditorSubsystem::OnEndPIE(bool bIsSimulating)
{
    if (DiscoveryTicker.IsValid()) FTSTicker::GetCoreTicker().RemoveTicker(DiscoveryTicker);
    DiscoveryTicker.Reset();
    RemovePIEComponents();
}

bool UFalloutWeaponTuningEditorSubsystem::DiscoverPIEControllers(float DeltaTime)
{
    bool bHasPIEWorld = false;
    if (!GEngine) return false;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (!World || World->WorldType != EWorldType::PIE) continue;
        bHasPIEWorld = true;
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* Controller = It->Get();
            if (!Controller || !Controller->IsLocalController() || Controller->FindComponentByClass<UFalloutWeaponTuningControllerComponent>()) continue;
            UFalloutWeaponTuningControllerComponent* TuningComponent = NewObject<UFalloutWeaponTuningControllerComponent>(Controller);
            if (!TuningComponent) continue;
            Controller->AddInstanceComponent(TuningComponent);
            TuningComponent->RegisterComponent();
            TuningComponent->StartTuning();
        }
    }
    return bHasPIEWorld;
}

void UFalloutWeaponTuningEditorSubsystem::RemovePIEComponents()
{
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (!World || World->WorldType != EWorldType::PIE) continue;
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* Controller = It->Get();
            if (UFalloutWeaponTuningControllerComponent* TuningComponent =
                Controller ? Controller->FindComponentByClass<UFalloutWeaponTuningControllerComponent>() : nullptr)
            {
                TuningComponent->ShutdownTuning();
                Controller->RemoveInstanceComponent(TuningComponent);
                TuningComponent->DestroyComponent();
            }
        }
    }
}

#endif
