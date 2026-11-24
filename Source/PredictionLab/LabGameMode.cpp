#include "LabGameMode.h"
#include "LabCharacter.h"
#include "LabHUD.h"
ALabGameMode::ALabGameMode()
{
    DefaultPawnClass = ALabCharacter::StaticClass();
    HUDClass = ALabHUD::StaticClass();
}
