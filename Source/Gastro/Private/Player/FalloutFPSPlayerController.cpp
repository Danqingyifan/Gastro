#include "Player/FalloutFPSPlayerController.h"
#include "Player/FalloutFPSCameraManager.h"
#include "Character/FalloutCharacter.h"
#include "Input/FalloutFPSInputConfig.h"
#include "Input/GastroAbilityInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

AFalloutFPSPlayerController::AFalloutFPSPlayerController()
{
    PlayerCameraManagerClass = AFalloutFPSCameraManager::StaticClass();
    AbilityInput = CreateDefaultSubobject<UGastroAbilityInputComponent>(TEXT("AbilityInput"));
}

void AFalloutFPSPlayerController::ConfigureFPSInput(UEnhancedInputComponent* Input, const UFalloutFPSInputConfig* Config)
{
    ClearFPSInput();
    if (!Config || !Input) return;
    AbilityInput->BindInputs(Input, Config);
    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        ActiveMappingContext = Config->MappingContext;
        if (ActiveMappingContext) Subsystem->AddMappingContext(ActiveMappingContext, 0);
    }
}
void AFalloutFPSPlayerController::ClearFPSInput()
{
    AbilityInput->UnbindInputs();
    if (ActiveMappingContext)
    {
        if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
            Subsystem->RemoveMappingContext(ActiveMappingContext);
        ActiveMappingContext = nullptr;
    }
    if (auto* FPSCharacter = Cast<AFalloutCharacter>(GetPawn())) FPSCharacter->ResetActionIntent();
}
void AFalloutFPSPlayerController::ApplyMouseLook(const FVector2D& Delta)
{
    if (IsLookInputIgnored() || !FMath::IsFinite(Delta.X) || !FMath::IsFinite(Delta.Y)) return;
    FRotator Rotation = GetControlRotation();
    Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + Delta.X * MouseSensitivity);
    const float MinPitch = PlayerCameraManager ? PlayerCameraManager->ViewPitchMin : -85.f;
    const float MaxPitch = PlayerCameraManager ? PlayerCameraManager->ViewPitchMax : 85.f;
    // Clamp before normalizing: an extreme delta must hit the limit instead of wrapping past it.
    Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch) + Delta.Y * MouseSensitivity, MinPitch, MaxPitch);
    Rotation.Roll = 0.f;
    SetControlRotation(Rotation);
}
void AFalloutFPSPlayerController::FlushPressedKeys()
{
    Super::FlushPressedKeys();
    AbilityInput->ReleaseAllInputs();
    if (auto* FPSCharacter = Cast<AFalloutCharacter>(GetPawn())) FPSCharacter->ResetActionIntent();
}
void AFalloutFPSPlayerController::SetPawn(APawn* InPawn)
{
    if (GetPawn() != InPawn) ClearFPSInput();
    Super::SetPawn(InPawn);
}
void AFalloutFPSPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearFPSInput();
    Super::EndPlay(Reason);
}
