#include "LabMovement.h"
#include "LabCharacter.h"

float ULabMovement::GetMaxSpeed() const
{
    const bool bAuthority = CharacterOwner && CharacterOwner->HasAuthority();
    const bool bDenied = bAuthority ? bServerSprintLocked : bOwnerSprintDenied;
    // The server independently validates the movement mode and its own policy.
    // Max speed is a constant, not a client RPC argument.
    if (MovementMode == MOVE_Walking && !IsCrouching())
        return bWantsSprint && !bDenied ? 900.f : 450.f;
    return Super::GetMaxSpeed();
}

void ULabMovement::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);
    bWantsSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
    if (CharacterOwner && CharacterOwner->HasAuthority() && bWantsSprint && bServerSprintLocked)
    {
        ALabCharacter* Lab = Cast<ALabCharacter>(CharacterOwner);
        if (Lab && !bOwnerSprintDenied)
        {
            bOwnerSprintDenied = true; // notify once until the server rule is toggled
            Lab->ClientSprintPolicy(true);
            UE_LOG(LogTemp, Display, TEXT("LAB event=server_reject reason=sprint_locked pawn=%s"), *GetNameSafe(CharacterOwner));
        }
    }
}

FNetworkPredictionData_Client* ULabMovement::GetPredictionData_Client() const
{
    if (!ClientPredictionData)
        const_cast<ULabMovement*>(this)->ClientPredictionData = new FPredictionData_Lab(*this);
    return ClientPredictionData;
}

void ULabMovement::OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData,
    float TimeStamp, FVector NewLocation, FVector NewVelocity,
    FMovementBaseInterfaceData* NewMovementBaseInterfaceData, FName NewBaseBoneName,
    bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection)
{
    ++CorrectionCount;
    UE_LOG(LogTemp, Display, TEXT("LAB event=cmc_correction count=%d saved=%d error_cm=%.2f"),
        CorrectionCount, ClientData.SavedMoves.Num(), FVector::Dist(CharacterOwner->GetActorLocation(), NewLocation));
    Super::OnClientCorrectionReceived(ClientData, TimeStamp, NewLocation, NewVelocity,
        NewMovementBaseInterfaceData, NewBaseBoneName, bHasBase, bBaseRelativePosition,
        ServerMovementMode, ServerGravityDirection);
}

bool ULabMovement::ClientUpdatePositionAfterServerUpdate()
{
    // PrepMoveFor must restore historical flags during replay, but those flags
    // must not overwrite the current live input after replay finishes.
    const bool bLiveSprint = bWantsSprint;
    const bool bReplayed = Super::ClientUpdatePositionAfterServerUpdate();
    bWantsSprint = bLiveSprint;
    return bReplayed;
}

void FSavedMove_Lab::Clear()
{
    Super::Clear();
    bSavedSprint = false;
}
uint8 FSavedMove_Lab::GetCompressedFlags() const
{
    return Super::GetCompressedFlags() | (bSavedSprint ? FLAG_Custom_0 : 0);
}
bool FSavedMove_Lab::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
    if (bSavedSprint != static_cast<const FSavedMove_Lab*>(NewMove.Get())->bSavedSprint)
        return false;
    return Super::CanCombineWith(NewMove, Character, MaxDelta);
}
void FSavedMove_Lab::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
    FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
    bSavedSprint = CastChecked<ULabMovement>(Character->GetCharacterMovement())->bWantsSprint;
}
void FSavedMove_Lab::PrepMoveFor(ACharacter* Character)
{
    Super::PrepMoveFor(Character);
    auto* Move = CastChecked<ULabMovement>(Character->GetCharacterMovement());
    Move->bWantsSprint = bSavedSprint;
    ++Move->ReplayCount;
    // Do not restore bOwnerSprintDenied from saved moves: server policy wins on replay.
}
