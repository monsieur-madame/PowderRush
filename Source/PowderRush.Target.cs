using UnrealBuildTool;
using System.Collections.Generic;

public class PowderRushTarget : TargetRules
{
    public PowderRushTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("PowderRush");
    }
}
