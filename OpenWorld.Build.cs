// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OpenWorld : ModuleRules
{
	public OpenWorld(ReadOnlyTargetRules Target) : base(Target)
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
			"ZoneGraph",
			"Slate",
			"MassEntity",
			"MassSimulation",
			"MassCommon",
			"GameplayTags",
			"MassNavigation",
			"MassLOD",
			"NavigationSystem",
			"NetCore",
			"MassMovement",
			"MassReplication",
			"MassAIReplication",
			"MassCrowd",
			"MassAIBehavior"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "MassSpawner", "MassZoneGraphNavigation", "MassActors", "MassRepresentation", "MassCrowd", "AnimGraphRuntime" });

		PublicIncludePaths.AddRange(new string[] {
			"OpenWorld",
			"OpenWorld/Variant_Platforming",
			"OpenWorld/Variant_Platforming/Animation",
			"OpenWorld/Variant_Combat",
			"OpenWorld/Variant_Combat/AI",
			"OpenWorld/Variant_Combat/Animation",
			"OpenWorld/Variant_Combat/Gameplay",
			"OpenWorld/Variant_Combat/Interfaces",
			"OpenWorld/Variant_Combat/UI",
			"OpenWorld/Variant_SideScrolling",
			"OpenWorld/Variant_SideScrolling/AI",
			"OpenWorld/Variant_SideScrolling/Gameplay",
			"OpenWorld/Variant_SideScrolling/Interfaces",
			"OpenWorld/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
