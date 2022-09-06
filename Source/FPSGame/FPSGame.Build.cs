// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class FPSGame : ModuleRules
{
	public FPSGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "GeometryCore", "MeshDescription" , "GeometryFramework", "MeshConversion", "StaticMeshDescription", "MyDynamicMesh", "MyGeometryAlgorithms" });


        //PrivateDependencyModuleNames.AddRange(new string[] { "MyDynamicMesh" });
        
        
        /*PublicIncludePaths.AddRange(new string[] { 
            "$(EngineDir)/Plugins/Runtime/GeometryProcessing/Source/DynamicMesh/Public",
			"$(EngineDir)/Plugins/Runtime/GeometryProcessing/Source/DynamicMesh/Public/ShapeApproximation",
            Path.Combine(ModuleDirectory,"Plugins/MyShapeApproximation/Source/MyShapeApproximation/Public"),
            "$(ModuleDir)/Plugins/MyShapeApproximation/Source/MyShapeApproximation/Public"});
        //PrivateDependencyModuleNames.Add("GeometricObjects");

        PrivateIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Plugins/MyShapeApproximation/Source/MyShapeApproximation/Private") });

        //DynamicallyLoadedModuleNames.AddRange(new string[] { "MyShapeApproximation" });

        */
    }
}
