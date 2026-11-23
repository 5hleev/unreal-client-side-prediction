#include "LabCharacter.h"
#include "LabMovement.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

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
    UE_LOG(LogTemp, Display, TEXT("LAB event=input id=%u csp=%d time=%.6f"), InputSequence, bPredictionEnabled, PressTime);
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
    if (IsLocallyControlled() && bResponsePending && LabMovement()->bWantsSprint &&
        !LabMovement()->bOwnerSprintDenied && LabMovement()->IsMovingOnGround() && GetVelocity().Size2D() > 475.f)
    {
        LastResponseMs = float((FPlatformTime::Seconds() - PressTime) * 1000.0);
        bResponsePending = false;
        UE_LOG(LogTemp, Display, TEXT("LAB event=response id=%u csp=%d response_ms=%.3f"),
            InputSequence, bPredictionEnabled, LastResponseMs);
    }
}
