using UnrealBuildTool;

public class Gastro : ModuleRules
{
    public Gastro(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] 
            { 
                "Core","CoreUObject", "Engine",
                "InputCore", "EnhancedInput",
                "GameplayAbilities", "GameplayTags", "GameplayTasks",
                "GastroFramework"
            }
        );  
    }
}
