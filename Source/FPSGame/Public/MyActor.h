// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
#include "ShapeApproximation/MeshSimpleShapeApproximation.h"
//#include "MyPrimitiveShapeApproximation.h"
#include "MeshDescription.h"
#include "UDynamicMesh.h"
#include "StaticMeshAttributes.h"
#include "DynamicMeshToMeshDescription.h"
#include "Engine/StaticMeshActor.h"
#include "Generators/GridBoxMeshGenerator.h"
#include "Generators/SphereGenerator.h"
#include "Generators/CapsuleGenerator.h"
#include "MyActor.generated.h"

UCLASS()
class FPSGAME_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();
	
	void Initialize(const TArray<UE::Geometry::FDynamicMesh3>& InitDynMesh, const TArray<FTransform>& Transforms);
	void DrawInitializedDynMesh();
	void AddMesh(const UE::Geometry::FDynamicMesh3& AddMesh);
	void Approximate();
	void DrawPrimitives();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Disappear();
	void UpdatePtrArray();
	void PrintDynMeshProperties(const UE::Geometry::FDynamicMesh3& DynMesh, FString OutputStr = FString(""));
	void PrintDynMeshProperties(const TArray<UE::Geometry::FDynamicMesh3>& DynMeshArr);

	AStaticMeshActor* MakeStaticMeshActorFromDynamicMesh(const FName Name, const UE::Geometry::FDynamicMesh3* DynMesh, const FTransform& RelativeTrans);
	void DrawBoxes();
	void DrawCapsules();
	void DrawSpheres();
	void DrawConvexes();

	void Normalize(const TArray<FVector3d>& Scaling);
	void RelatviveTransform(const TArray<FTransform>& Transform);

	/** Dynamic Mesh */
	TArray<const UE::Geometry::FDynamicMesh3*>* DynamicSourceMeshesPtrArr;

	TArray<UE::Geometry::FDynamicMesh3> DynamicSourceMeshesObjArr;

	//UE::Geometry::FMyPrimitiveShapeApproximation* ShapeApprox;
	UE::Geometry::FMeshSimpleShapeApproximation* ShapeApprox;;
	UE::Geometry::FSimpleShapeSet3d* ShapePrimitiveSet;

	TArray<AStaticMeshActor*> OwnedStaticMeshActors;
	TArray<UStaticMesh*> StaticPrimitives;

	FDynamicMeshToMeshDescription Converter;

	
	UPROPERTY()
	UStaticMeshComponent* Root;

	

	bool IsApproximated;

	bool IsInitialized;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
