using UnrealBuildTool;
using System.Collections.Generic;

public class PowderRushEditorTarget : TargetRules
{
    public PowderRushEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("PowderRush");
    }
}
