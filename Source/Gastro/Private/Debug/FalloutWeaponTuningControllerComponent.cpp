#include "Debug/FalloutWeaponTuningControllerComponent.h"

#include "Components/InputComponent.h"
#include "Debug/FalloutWeaponTuningWidget.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

UFalloutWeaponTuningControllerComponent::UFalloutWeaponTuningControllerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFalloutWeaponTuningControllerComponent::BeginPlay()
{
    Super::BeginPlay();
    StartTuning();
}

void UFalloutWeaponTuningControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ShutdownTuning();
    Super::EndPlay(EndPlayReason);
}

void UFalloutWeaponTuningControllerComponent::StartTuning()
{
    if (bStarted) return;
    Controller = Cast<APlayerController>(GetOwner());
    if (!Controller || !Controller->IsLocalController()) return;

    bStarted = true;
    DebugInputComponent = NewObject<UInputComponent>(Controller, TEXT("TTFWeaponTuningInput"));
    if (DebugInputComponent)
    {
        DebugInputComponent->RegisterComponent();
        FInputKeyBinding Binding(FInputChord(EKeys::F1), IE_Pressed);
        Binding.KeyDelegate.GetDelegateForManualSet().BindUObject(this, &ThisClass::TogglePanel);
        DebugInputComponent->KeyBindings.Add(MoveTemp(Binding));
        Controller->PushInputComponent(DebugInputComponent);
    }
}

void UFalloutWeaponTuningControllerComponent::ShutdownTuning()
{
    ClosePanel();
    if (Controller)
    {
        if (DebugInputComponent) Controller->PopInputComponent(DebugInputComponent);
    }
    if (DebugInputComponent) DebugInputComponent->DestroyComponent();
    DebugInputComponent = nullptr;
    Controller = nullptr;
    bStarted = false;
}

void UFalloutWeaponTuningControllerComponent::TogglePanel()
{
    if (!Controller) return;
    if (Panel)
    {
        ClosePanel();
        return;
    }

    TSubclassOf<UFalloutWeaponTuningWidget> WidgetClass = LoadClass<UFalloutWeaponTuningWidget>(nullptr,
        TEXT("/Game/ThroughTheFallout/Debug/UI/WBP_TTFWeaponTuningPanel.WBP_TTFWeaponTuningPanel_C"));
    if (!WidgetClass) WidgetClass = UFalloutWeaponTuningWidget::StaticClass();
    Panel = CreateWidget<UFalloutWeaponTuningWidget>(Controller, WidgetClass);
    if (!Panel) return;
    Panel->AddToViewport(10000);
    Panel->SetAnchorsInViewport(FAnchors(0.f, 0.f, 0.f, 0.f));
    Panel->SetAlignmentInViewport(FVector2D::ZeroVector);
    Panel->SetPositionInViewport(FVector2D(24.f, 24.f), false);

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    Controller->SetInputMode(InputMode);
    Controller->bShowMouseCursor = true;
}

void UFalloutWeaponTuningControllerComponent::ClosePanel()
{
    if (!Panel) return;
    Panel->RemoveFromParent();
    Panel = nullptr;
    if (Controller)
    {
        Controller->SetInputMode(FInputModeGameOnly());
        Controller->bShowMouseCursor = false;
    }
}
