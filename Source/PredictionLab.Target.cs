using UnrealBuildTool;
using System.Collections.Generic;
public class PredictionLabTarget : TargetRules
{
    public PredictionLabTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        WindowsPlatform.CompilerVersion = "Latest";
        ExtraModuleNames.Add("PredictionLab");
    }
}
