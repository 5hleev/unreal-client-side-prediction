#include "LabHUD.h"
#include "LabCharacter.h"
#include "LabMovement.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Canvas.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
void ALabHUD::DrawHUD()
{
    Super::DrawHUD();
    const auto* Character = Cast<ALabCharacter>(GetOwningPlayerController()->GetPawn());
    if (!Character || !Canvas) return;
    DrawRect(FLinearColor(0, 0, 0, 0.75f), 12, 12, 810, 155);
    const auto* Move = Character->LabMovement();
    FString Profile = TEXT("manual");
    FParse::Value(FCommandLine::Get(), TEXT("LabProfile="), Profile);
    DrawText(FString::Printf(TEXT("PredictionLab UE5.8 | %s | CSP %s | %s"),
        Character->HasAuthority() ? TEXT("AUTHORITY / HOST") : TEXT("AUTONOMOUS / CLIENT"),
        Character->IsPredictionEnabled() ? TEXT("ON") : TEXT("OFF"), *Profile), FColor::Cyan, 24, 22);
    DrawText(TEXT("WASD move | Mouse look | Space jump | Shift sprint | F2 CSP (release Shift first)"), FColor::White, 24, 48);
    DrawText(TEXT("F3 toggle server sprint lock (demo) | ~ console | Esc release via editor"), FColor::White, 24, 72);
    DrawText(FString::Printf(TEXT("Speed %.0f cm/s | max %.0f | intent %d | known denial %d"),
        Character->GetVelocity().Size2D(), Move->GetMaxSpeed(), Move->bWantsSprint, Move->bOwnerSprintDenied), FColor::Yellow, 24, 96);
    DrawText(FString::Printf(TEXT("Response %.1f ms | CMC corrections %d | replayed moves %d"),
        Character->LastResponseMs, Move->CorrectionCount, Move->ReplayCount), FColor::White, 24, 120);
}
