namespace UnrealBuildTool.Rules
{
    using System.IO;

    public class paho_c_sync : ModuleRules
    {
        public paho_c_sync(ReadOnlyTargetRules Target) : base(Target)
        {
    		Type = ModuleType.External;
            bEnableUndefinedIdentifierWarnings = false;
            bEnableExceptions = true;

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                bUseRTTI = true;

                PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Win64", "include"));
                PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Win64", "lib", "paho-mqtt3cs-static.lib"));
            }
        }
    }
}
