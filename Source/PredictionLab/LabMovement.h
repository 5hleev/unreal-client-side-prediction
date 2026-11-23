#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LabMovement.generated.h"

UCLASS()
class PREDICTIONLAB_API ULabMovement : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    bool bWantsSprint = false;
    // Authority-only game rule. Never read from client move data.
    bool bServerSprintLocked = false;
    // Owner learns rejection through a reliable server->owner RPC.
    bool bOwnerSprintDenied = false;
    virtual float GetMaxSpeed() const override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class FSavedMove_Lab : public FSavedMove_Character
{
public:
    bool bSavedSprint = false;
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
    virtual void SetMoveFor(ACharacter* Character, float DeltaTime, FVector const& NewAccel,
        FNetworkPredictionData_Client_Character& ClientData) override;
    virtual void PrepMoveFor(ACharacter* Character) override;
};

class FPredictionData_Lab : public FNetworkPredictionData_Client_Character
{
public:
    explicit FPredictionData_Lab(const UCharacterMovementComponent& Movement)
        : FNetworkPredictionData_Client_Character(Movement) {}
    virtual FSavedMovePtr AllocateNewMove() override { return FSavedMovePtr(new FSavedMove_Lab()); }
};
