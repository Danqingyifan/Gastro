#include "Player/FalloutFPSHUD.h"
#include "Character/FalloutCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

void AFalloutFPSHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !PlayerOwner || !Cast<AFalloutCharacter>(PlayerOwner->GetPawn())) return;
    const float X = Canvas->ClipX * 0.5f;
    const float Y = Canvas->ClipY * 0.5f;
    DrawLine(X - 7, Y, X - 3, Y, FLinearColor::White);
    DrawLine(X + 3, Y, X + 7, Y, FLinearColor::White);
    DrawLine(X, Y - 7, X, Y - 3, FLinearColor::White);
    DrawLine(X, Y + 3, X, Y + 7, FLinearColor::White);
    if (ShouldDisplayDebug(TEXT("FPS3C")))
    {
        const auto* FPSCharacter = CastChecked<AFalloutCharacter>(PlayerOwner->GetPawn());
        DrawText(FString::Printf(TEXT("FPS3C | speed %.1f cm/s | %s | eye %.1f cm"), FPSCharacter->GetVelocity().Size2D(),
            FPSCharacter->bIsCrouched ? TEXT("CROUCH") : FPSCharacter->GetCharacterMovement()->IsFalling() ? TEXT("AIR") : TEXT("GROUND"),
            FPSCharacter->GetVisualEyeHeight()), FLinearColor::White, 24, Canvas->ClipY - 45, GEngine->GetSmallFont());
    }
}
