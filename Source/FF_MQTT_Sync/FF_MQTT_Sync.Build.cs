// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class FF_MQTT_Sync : ModuleRules
{
	public FF_MQTT_Sync(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

		// You need to add openssl ios libraries or FF_OpenSSL (private) plugin for iOS support.

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "OpenSSL",
                "paho_c",
                "paho_c_sync",
            }
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities",
                "JsonBlueprintUtilities",
            }
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}
