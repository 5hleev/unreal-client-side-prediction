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
    int32 CorrectionCount = 0;
    int32 ReplayCount = 0;
    virtual float GetMaxSpeed() const override;
    virtual void UpdateFromCompressedFlags(uint8 Flags) override;
    virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
    virtual void OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData,
        float TimeStamp, FVector NewLocation, FVector NewVelocity,
        FMovementBaseInterfaceData* NewMovementBaseInterfaceData, FName NewBaseBoneName,
        bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;
};

class FSavedMove_Lab : public FSavedMove_Character
{
public:
    using Super = FSavedMove_Character;
    bool bSavedSprint = false;
    virtual void Clear() override;
    virtual uint8 GetCompressedFlags() const override;
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
    virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
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
