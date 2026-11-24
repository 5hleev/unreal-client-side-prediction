#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LabHUD.generated.h"
UCLASS()
class PREDICTIONLAB_API ALabHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};
