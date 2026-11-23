using UnrealBuildTool;
using System.Collections.Generic;
public class PredictionLabEditorTarget : TargetRules
{
    public PredictionLabEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("PredictionLab");
    }
}
