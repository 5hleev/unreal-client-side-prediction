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
void FSavedMove_Lab::SetMoveFor(ACharacter* Character, float DeltaTime, FVector const& NewAccel,
    FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(Character, DeltaTime, NewAccel, ClientData);
    bSavedSprint = CastChecked<ULabMovement>(Character->GetCharacterMovement())->bWantsSprint;
}
void FSavedMove_Lab::PrepMoveFor(ACharacter* Character)
{
    Super::PrepMoveFor(Character);
    CastChecked<ULabMovement>(Character->GetCharacterMovement())->bWantsSprint = bSavedSprint;
    // Do not restore bOwnerSprintDenied from saved moves: server policy wins on replay.
}
