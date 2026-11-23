#include "LabCharacter.h"
#include "LabMovement.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"

ALabCharacter::ALabCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<ULabMovement>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.25f;
    auto* Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Arm->SetupAttachment(RootComponent);
    Arm->TargetArmLength = 550.f;
    Arm->bUsePawnControlRotation = true;
    Arm->SocketOffset = FVector(0, 0, 100);
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    Camera->SetupAttachment(Arm);
    auto* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleBody"));
    Body->SetupAttachment(RootComponent);
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetRelativeScale3D(FVector(0.65f, 0.65f, 1.5f));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
}
ULabMovement* ALabCharacter::LabMovement() const { return CastChecked<ULabMovement>(GetCharacterMovement()); }
void ALabCharacter::BeginPlay()
{
    Super::BeginPlay();
    bAutoDemo = FParse::Param(FCommandLine::Get(), TEXT("LabAuto"));
}
void ALabCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis("Forward", this, &ALabCharacter::Forward);
    Input->BindAxis("Right", this, &ALabCharacter::Right);
    Input->BindAxis("Turn", this, &ALabCharacter::Turn);
    Input->BindAxis("Look", this, &ALabCharacter::Look);
    Input->BindAction("Sprint", IE_Pressed, this, &ALabCharacter::SprintPressed);
    Input->BindAction("Sprint", IE_Released, this, &ALabCharacter::SprintReleased);
    Input->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    Input->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    Input->BindAction("TogglePrediction", IE_Pressed, this, &ALabCharacter::TogglePrediction);
    Input->BindAction("ToggleServerLock", IE_Pressed, this, &ALabCharacter::ToggleServerLock);
}
void ALabCharacter::Forward(float Value)
{
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::X), Value);
}
void ALabCharacter::Right(float Value)
{
    if (Controller) AddMovementInput(FRotationMatrix(FRotator(0, Controller->GetControlRotation().Yaw, 0)).GetUnitAxis(EAxis::Y), Value);
}
void ALabCharacter::Turn(float Value) { AddControllerYawInput(Value); }
void ALabCharacter::Look(float Value) { AddControllerPitchInput(Value); }
void ALabCharacter::SprintPressed()
{
    bSprintHeld = true;
    ++InputSequence;
    PressTime = FPlatformTime::Seconds();
    LastResponseMs = -1.f;
    bResponsePending = true;
    bBaselineApproved = false;
    UE_LOG(LogTemp, Display, TEXT("LAB event=input id=%u csp=%d time=%.6f pre_speed=%.1f"),
        InputSequence, bPredictionEnabled, PressTime, GetVelocity().Size2D());
    if (bPredictionEnabled) LabMovement()->bWantsSprint = true;
    else ServerBaselineSprint(true, InputSequence);
}
void ALabCharacter::SprintReleased()
{
    bSprintHeld = false;
    bResponsePending = false;
    ++InputSequence;
    if (bPredictionEnabled) LabMovement()->bWantsSprint = false;
    else ServerBaselineSprint(false, InputSequence);
}
void ALabCharacter::TogglePrediction()
{
    if (bSprintHeld || LabMovement()->bWantsSprint) return;
    bPredictionEnabled = !bPredictionEnabled;
    UE_LOG(LogTemp, Display, TEXT("LAB event=mode csp=%d"), bPredictionEnabled);
}
void ALabCharacter::ServerBaselineSprint_Implementation(bool bPressed, uint32 RequestId)
{
    // Reliable requests arrive in order. No speed, position or client time is accepted.
    LastBaselineRequest = RequestId;
    const bool bAccepted = bPressed && !LabMovement()->bServerSprintLocked && LabMovement()->IsMovingOnGround();
    UE_LOG(LogTemp, Display, TEXT("LAB event=baseline_server id=%u accepted=%d"), RequestId, bAccepted);
    ClientBaselineSprint(bAccepted, LastBaselineRequest);
}
void ALabCharacter::ClientBaselineSprint_Implementation(bool bAccepted, uint32 RequestId)
{
    if (bPredictionEnabled || RequestId != InputSequence) return;
    bBaselineApproved = bAccepted && bSprintHeld;
    LabMovement()->bWantsSprint = bAccepted && bSprintHeld;
    UE_LOG(LogTemp, Display, TEXT("LAB event=baseline_ack id=%u accepted=%d"), RequestId, bAccepted);
}
void ALabCharacter::ToggleServerLock() { ServerToggleSprintLock(); }
void ALabCharacter::ServerToggleSprintLock_Implementation()
{
    // Development-only demonstration control. Remove this RPC from a real game.
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now - LastPolicyToggle < 1.0) return;
    LastPolicyToggle = Now;
    auto* Move = LabMovement();
    Move->bServerSprintLocked = !Move->bServerSprintLocked;
    Move->bOwnerSprintDenied = false;
    if (!Move->bServerSprintLocked) ClientSprintPolicy(false);
    UE_LOG(LogTemp, Display, TEXT("LAB event=server_policy locked=%d pawn=%s"), Move->bServerSprintLocked, *GetName());
}
void ALabCharacter::ClientSprintPolicy_Implementation(bool bDenied)
{
    LabMovement()->bOwnerSprintDenied = bDenied;
    UE_LOG(LogTemp, Display, TEXT("LAB event=owner_policy denied=%d"), bDenied);
}
void ALabCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bAutoDemo && IsLocallyControlled() && !HasAuthority()) AutoDemo(DeltaSeconds);
    if (HasAuthority() && GetWorld()->GetTimeSeconds() >= NextTelemetry)
    {
        NextTelemetry = GetWorld()->GetTimeSeconds() + 1.0;
        UE_LOG(LogTemp, Display, TEXT("LAB event=authority pawn=%s speed=%.1f max=%.1f locked=%d role=%d"),
            *GetName(), GetVelocity().Size2D(), LabMovement()->GetMaxSpeed(), LabMovement()->bServerSprintLocked, int32(GetLocalRole()));
    }
    if (IsLocallyControlled() && bResponsePending && (bPredictionEnabled || bBaselineApproved) && LabMovement()->bWantsSprint &&
        !LabMovement()->bOwnerSprintDenied && LabMovement()->IsMovingOnGround() && GetVelocity().Size2D() > 475.f)
    {
        LastResponseMs = float((FPlatformTime::Seconds() - PressTime) * 1000.0);
        bResponsePending = false;
        UE_LOG(LogTemp, Display, TEXT("LAB event=response id=%u csp=%d response_ms=%.3f"),
            InputSequence, bPredictionEnabled, LastResponseMs);
    }
}

void ALabCharacter::AutoDemo(float DeltaSeconds)
{
    // Real autonomous-proxy input for reproducible experiments. Opt-in via -LabAuto.
    // No position writes, fake network events or synthetic measurement values.
    AutoElapsed += DeltaSeconds;
    if (AutoElapsed < 3.0)
    {
        auto* Player = Cast<APlayerController>(Controller);
        if (Player && !OverviewCamera)
        {
            const FVector Eye(-3000, -2000, 1800);
            auto* Camera = GetWorld()->SpawnActor<ACameraActor>(Eye, (FVector(-1500, 0, 100) - Eye).Rotation());
            OverviewCamera = Camera;
            Player->SetViewTarget(Camera);
        }
        if (!bOverviewCaptured && AutoElapsed >= 1.7)
        {
            FString Directory;
            if (!FParse::Value(FCommandLine::Get(), TEXT("LabMedia="), Directory))
                Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LabMedia"));
            FScreenshotRequest::RequestScreenshot(FPaths::Combine(Directory, TEXT("two_players.png")), false, false);
            bOverviewCaptured = true;
        }
        if (Player && AutoElapsed >= 2.5) Player->SetViewTarget(this);
        return;
    }
    const int32 Trial = int32((AutoElapsed - 3.0) / 4.0);
    const double Within = FMath::Fmod(AutoElapsed - 3.0, 4.0);
    if (Trial >= 23)
    {
        UE_LOG(LogTemp, Display, TEXT("LAB event=auto_complete corrections=%d replays=%d"),
            LabMovement()->CorrectionCount, LabMovement()->ReplayCount);
        FPlatformMisc::RequestExit(false);
        bAutoDemo = false;
        return;
    }
    if (Trial != AutoTrial)
    {
        if (bSprintHeld) SprintReleased();
        AutoTrial = Trial;
        bAutoPressed = false;
        bAutoCaptured = false;
        bAutoBeforeCaptured = false;
        // 10 baseline trials, 10 predicted trials, rejection, unlock, recovery.
        bPredictionEnabled = Trial >= 10;
        if (Trial == 20 || Trial == 21) ServerToggleSprintLock();
        UE_LOG(LogTemp, Display, TEXT("LAB event=trial trial=%d csp=%d"), Trial, bPredictionEnabled);
    }
    const float Sign = Trial % 2 == 0 ? 1.f : -1.f;
    if (Within < 2.8) AddMovementInput(FVector::ForwardVector, Sign);
    if (!bAutoPressed && Within >= 1.3)
    {
        SprintPressed();
        bAutoPressed = true;
    }
    if (bSprintHeld && Within >= 2.7) SprintReleased();
    if (!bAutoBeforeCaptured && Trial == 20 && Within >= 1.45)
    {
        FString Directory;
        if (!FParse::Value(FCommandLine::Get(), TEXT("LabMedia="), Directory))
            Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LabMedia"));
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Directory, TEXT("correction_before.png")), false, false);
        bAutoBeforeCaptured = true;
    }
    if (!bAutoCaptured && Within >= 2.2 && (Trial == 1 || Trial == 11 || Trial == 20))
    {
        FString Name = Trial == 1 ? TEXT("baseline_no_prediction.png") :
            Trial == 11 ? TEXT("prediction_enabled.png") : TEXT("correction_example.png");
        FString Directory;
        if (!FParse::Value(FCommandLine::Get(), TEXT("LabMedia="), Directory))
            Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("LabMedia"));
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Directory, Name), false, false);
        bAutoCaptured = true;
    }
}
