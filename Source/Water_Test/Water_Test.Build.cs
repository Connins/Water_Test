// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Water_Test : ModuleRules
{
	public Water_Test(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Water",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Water_Test",
			"Water_Test/Boat",
			"Water_Test/Variant_Platforming",
			"Water_Test/Variant_Platforming/Animation",
			"Water_Test/Variant_Combat",
			"Water_Test/Variant_Combat/AI",
			"Water_Test/Variant_Combat/Animation",
			"Water_Test/Variant_Combat/Gameplay",
			"Water_Test/Variant_Combat/Interfaces",
			"Water_Test/Variant_Combat/UI",
			"Water_Test/Variant_SideScrolling",
			"Water_Test/Variant_SideScrolling/AI",
			"Water_Test/Variant_SideScrolling/Gameplay",
			"Water_Test/Variant_SideScrolling/Interfaces",
			"Water_Test/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
