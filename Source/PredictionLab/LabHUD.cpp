#include "LabHUD.h"
#include "LabCharacter.h"
#include "LabMovement.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
void ALabHUD::DrawHUD()
{
    Super::DrawHUD();
    const auto* Character = Cast<ALabCharacter>(GetOwningPlayerController()->GetPawn());
    if (!Character || !Canvas) return;
    DrawRect(FLinearColor(0, 0, 0, 0.75f), 12, 12, 810, 155);
    const auto* Move = Character->LabMovement();
    DrawText(FString::Printf(TEXT("PredictionLab | %s | CSP %s"),
        Character->HasAuthority() ? TEXT("AUTHORITY / HOST") : TEXT("AUTONOMOUS / CLIENT"),
        Character->IsPredictionEnabled() ? TEXT("ON") : TEXT("OFF")), FColor::Cyan, 24, 22);
    DrawText(TEXT("WASD move | Mouse look | Space jump | Shift sprint | F2 CSP (release Shift first)"), FColor::White, 24, 48);
    DrawText(TEXT("F3 toggle server sprint lock (demo) | ~ console | Esc release via editor"), FColor::White, 24, 72);
    DrawText(FString::Printf(TEXT("Speed %.0f cm/s | max %.0f | intent %d | known denial %d"),
        Character->GetVelocity().Size2D(), Move->GetMaxSpeed(), Move->bWantsSprint, Move->bOwnerSprintDenied), FColor::Yellow, 24, 96);
    DrawText(FString::Printf(TEXT("Input -> speed >475 cm/s: %.1f ms (-1 = no sample)"), Character->LastResponseMs), FColor::White, 24, 120);
}
