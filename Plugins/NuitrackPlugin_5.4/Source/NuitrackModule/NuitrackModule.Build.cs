// Copyright 3DiVi 2024, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class NuitrackModule : ModuleRules
{
    public NuitrackModule(ReadOnlyTargetRules Target) : base(Target)
    {
        // We need to depend on Core in order to implement our game module in the next step
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "Niagara" });
        bAddDefaultIncludePaths = false;

        string LibraryPath = ModuleDirectory + "/lib/";
        string LibraryPrefix = "";
        string LibraryExtension = "";

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            LibraryPath += "win64/";
            LibraryExtension = ".lib";
        }
#if UE_5_0_OR_LATER

#else
        else if (Target.Platform == UnrealTargetPlatform.Win32)
        {
            LibraryPath += "win32/";
            LibraryExtension = ".lib";
        }
#endif
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            LibraryPath += "android/"; // "android-arm64/"
            LibraryPrefix = "lib";
            LibraryExtension = ".so";

            PublicDefinitions.Add("NUITRACK_NO_EXCEPTIONS");

            string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
#if UE_5_0_OR_LATER
            AdditionalPropertiesForReceipt.Add("AndroidPlugin",
                    Path.Combine(PluginPath, "VicoVR_APL.xml"));
#else
            AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin",
                    Path.Combine(PluginPath, "VicoVR_APL.xml")));
#endif
        }
        else
        {
            Console.Error.WriteLine("NuitrackModule does not support this target platform");
            Environment.Exit(1);
        }
        
        PublicIncludePaths.Add(ModuleDirectory + "/include");

        var libraries = new List<string> { "NuitrackManager", "libboost_filesystem-vc142-mt-x64-1_87" };
        foreach (var libraryName in libraries)
        {
            Console.WriteLine(LibraryPath + LibraryPrefix + libraryName + LibraryExtension);
            PublicAdditionalLibraries.Add(LibraryPath + LibraryPrefix + libraryName + LibraryExtension);
        }

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // string nuitrackManagerFileName = "NuitrackManager.dll";
            // string projectBinariesPath = Path.Combine(PluginDirectory, "..", "..", "Binaries", "Win64");

            // bool isMarketplace = Directory.Exists(Path.Combine(PluginDirectory, "..", "..", "Marketplace"));

            // if (isMarketplace)
            //     projectBinariesPath = Path.Combine(EngineDirectory, "..");

            // string sourceNuitrackManagerPath = Path.Combine(ModuleDirectory, "lib", "win64", nuitrackManagerFileName);
            // string targetNuitrackManagerPath = Path.Combine(projectBinariesPath, nuitrackManagerFileName);

            // if (!Directory.Exists(projectBinariesPath))
            //     Directory.CreateDirectory(projectBinariesPath);

            // if (!File.Exists(targetNuitrackManagerPath) || isMarketplace)
            //     File.Copy(sourceNuitrackManagerPath, targetNuitrackManagerPath, true);

            // PublicDelayLoadDLLs.Add(targetNuitrackManagerPath);
            // RuntimeDependencies.Add(targetNuitrackManagerPath);

            // string platformTargetPath = Path.GetFullPath(Path.Combine(PluginDirectory , "Binaries", "win64"));
            //
            // platformTargetPath = platformTargetPath.Replace("HostProject\\Plugins\\NuitrackPlugin\\", "");
            //
            // if (!Directory.Exists(platformTargetPath))
            //     Directory.CreateDirectory(platformTargetPath);
            //
            // Libraries = new List<string> { "NuitrackManager" };
            //
            // foreach (string LibraryName in Libraries)
            // {
            //     string targetFilePath = Path.Combine(platformTargetPath, LibraryName + ".dll");
            //     string fileSourcePath = LibraryPath + LibraryName + ".dll";
            //     if (!File.Exists(targetFilePath))
            //     {
            //         File.Copy(fileSourcePath, targetFilePath, true);
            //
            //         Console.WriteLine(LibraryPath + LibraryName + ".dll");
            //         Console.WriteLine(targetFilePath);
            //     }
            // }
        }
    }
}
