using UnrealBuildTool;
using System.Collections.Generic;
public class PredictionLabEditorTarget : TargetRules
{
    public PredictionLabEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        WindowsPlatform.CompilerVersion = "Latest";
        ExtraModuleNames.Add("PredictionLab");
    }
}
