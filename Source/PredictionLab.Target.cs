using UnrealBuildTool;
using System.Collections.Generic;
public class PredictionLabTarget : TargetRules
{
    public PredictionLabTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("PredictionLab");
    }
}
