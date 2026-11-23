#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LabCharacter.generated.h"

class ULabMovement;
UCLASS()
class PREDICTIONLAB_API ALabCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ALabCharacter(const FObjectInitializer& ObjectInitializer);
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float DeltaSeconds) override;
    ULabMovement* LabMovement() const;
    UFUNCTION(Client, Reliable) void ClientSprintPolicy(bool bDenied);
    bool IsPredictionEnabled() const { return bPredictionEnabled; }
    float LastResponseMs = -1.f;
private:
    void Forward(float Value);
    void Right(float Value);
    void Turn(float Value);
    void Look(float Value);
    void SprintPressed();
    void SprintReleased();
    void TogglePrediction();
    void ToggleServerLock();
    UFUNCTION(Server, Reliable) void ServerBaselineSprint(bool bPressed, uint32 RequestId);
    UFUNCTION(Client, Reliable) void ClientBaselineSprint(bool bAccepted, uint32 RequestId);
    UFUNCTION(Server, Reliable) void ServerToggleSprintLock();
    bool bPredictionEnabled = true;
    bool bSprintHeld = false;
    bool bResponsePending = false;
    uint32 InputSequence = 0;
    double PressTime = 0.0;
    double LastPolicyToggle = -10.0;
    uint32 LastBaselineRequest = 0;
};
