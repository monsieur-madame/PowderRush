using UnrealBuildTool;

public class PowderRush : ModuleRules
{
    public PowderRush(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "Niagara",
            "PhysicsCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CommonUI",
            "MetasoundEngine"
        });
    }
}
