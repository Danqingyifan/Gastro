using UnrealBuildTool;

public class GastroFramework : ModuleRules
{
    public GastroFramework(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "GameplayAbilities", "GameplayTags", "GameplayTasks"
        });
        PrivateDependencyModuleNames.AddRange(new[] { "EnhancedInput", "InputCore" });
    }
}
