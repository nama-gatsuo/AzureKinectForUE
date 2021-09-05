// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class AzureKinect : ModuleRules
{
	public AzureKinect(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string sdkPath = System.Environment.GetEnvironmentVariable("AZUREKINECT_SDK");
			string bodySdkPath = System.Environment.GetEnvironmentVariable("AZUREKINECT_BODY_SDK");

			PublicIncludePaths.AddRange(
				new string[] {
					Path.Combine(sdkPath, "sdk", "include"),
					Path.Combine(bodySdkPath, "sdk", "include")
				});

			PublicAdditionalLibraries.AddRange(
				new string[] {
					Path.Combine(sdkPath, "sdk", "windows-desktop", "amd64", "release", "lib", "k4a.lib"),
					Path.Combine(sdkPath, "sdk", "windows-desktop", "amd64", "release", "lib", "k4arecord.lib"),
					Path.Combine(bodySdkPath, "sdk", "windows-desktop", "amd64", "release", "lib", "k4abt.lib")
				});
			
			string depthEngineDllPath = Path.Combine(sdkPath, "sdk", "windows-desktop", "amd64", "release", "bin", "depthengine_2_0.dll");
			string k4aDllPath = Path.Combine(sdkPath, "sdk", "windows-desktop", "amd64", "release", "bin", "k4a.dll");
			string k4abtDllPath = Path.Combine(bodySdkPath, "sdk", "windows-desktop", "amd64", "release", "bin", "k4abt.dll");
			
			PublicDelayLoadDLLs.AddRange(
				new string[] {
					depthEngineDllPath,
					k4aDllPath,
					k4abtDllPath,
				});

			RuntimeDependencies.Add(depthEngineDllPath);
			RuntimeDependencies.Add(k4aDllPath);
			RuntimeDependencies.Add(k4abtDllPath);
		}

		PrivateIncludePaths.AddRange(
			new string[]
			{
				"AzureKinect/Private",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RenderCore",
				"RHI",
			});
		
	}
}
