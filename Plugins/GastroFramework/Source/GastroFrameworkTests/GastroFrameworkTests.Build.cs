using UnrealBuildTool;

public class GastroFrameworkTests : ModuleRules
{
    public GastroFrameworkTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "GastroFramework", "GameplayAbilities",
            "GameplayTags", "GameplayTasks", "EnhancedInput", "InputCore"
        });
    }
}
