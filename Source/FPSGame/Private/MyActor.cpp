// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"
#include "TimerManager.h"
#include <fstream>
//#include <BoxSphereGenerator.h>

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<UStaticMeshComponent>("Root");
	Root->SetMobility(EComponentMobility::Static);
	RootComponent = Root;
	
	//DynamicSourceMeshes = CreateDefaultSubobject<UE::Geometry::FDynamicMesh3>("DynamicMesh");	

	ShapeApprox = new UE::Geometry::FMeshSimpleShapeApproximation;
	//ShapeApprox = new UE::Geometry::FMyPrimitiveShapeApproximation;
	ShapePrimitiveSet = new UE::Geometry::FSimpleShapeSet3d;

	OwnedStaticMeshActors.Empty();
	StaticPrimitives.Empty();
	//StaticPrimitives = CreateDefaultSubobject<UStaticMeshComponent>("Array");
	
	DynamicSourceMeshesPtrArr = nullptr;
	IsInitialized = false;
	IsApproximated = false;

	Verbose = false;



	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Environment/Materials/M_Cube.M_Cube'"));
	if (Material.Object != NULL)
	{
		MaterialInterface = (UMaterialInterface*)Material.Object;
	}
	else
	{
		MaterialInterface = nullptr;
	}
}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AMyActor::Disappear, 5.0f, false);
	
}

void AMyActor::Disappear()
{
	delete ShapeApprox, ShapePrimitiveSet;
	if (DynamicSourceMeshesPtrArr != nullptr)
	{
		delete DynamicSourceMeshesPtrArr;
	}
	Destroy();
}

void AMyActor::Initialize(const TArray<UE::Geometry::FDynamicMesh3>& InitDynMesh, const TArray<FTransform>& Transforms)
{
	if (InitDynMesh.Num() == Transforms.Num())
	{
		DynamicSourceMeshesObjArr = InitDynMesh;
		RelatviveTransform(Transforms);
		UpdatePtrArray();
		IsInitialized = true;
		IsApproximated = false;
		if (Verbose)
		{
			PrintDynMeshProperties(InitDynMesh);
		}
	}
	else {
		FString UniversalString = "Both input arrays of AMyActor::Initialize() need to be of the same length";
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, UniversalString);
	}
}

void AMyActor::AddMesh(const UE::Geometry::FDynamicMesh3& AddMesh)
{
	DynamicSourceMeshesObjArr.Add(AddMesh);
	UpdatePtrArray();
	IsInitialized = true;
	IsApproximated = false;
}

void AMyActor::UpdatePtrArray()
{
	if (DynamicSourceMeshesPtrArr != nullptr)
	{
		delete DynamicSourceMeshesPtrArr;
	}
	TArray<UE::Geometry::FDynamicMesh3*> List;
	for (int32 i = 0; i < DynamicSourceMeshesObjArr.Num(); i++)
	{
		List.Add(&DynamicSourceMeshesObjArr[i]);
	}
	DynamicSourceMeshesPtrArr = new TArray<const UE::Geometry::FDynamicMesh3*>(List);
}


void AMyActor::Approximate()
{
	if (IsInitialized)
	{
		ShapeApprox->bDetectBoxes = true;
		ShapeApprox->bDetectCapsules = true;
		ShapeApprox->bDetectSpheres = true;
		ShapeApprox->bDetectConvexes = false;
		ShapeApprox->InitializeSourceMeshes(*DynamicSourceMeshesPtrArr);
		ShapeApprox->Generate_MinCostApproximation(*ShapePrimitiveSet);
		//ShapeApprox->Generate_MinVolume(*ShapePrimitiveSet);
		//ShapeApprox->Generate_OrientedBoxes(*ShapePrimitiveSet);
		//ShapeApprox->Generate_AlignedBoxes(*ShapePrimitiveSet);
		//ShapeApprox->Generate_MinimalSpheres(*ShapePrimitiveSet);
		//ShapeApprox->Generate_Capsules(*ShapePrimitiveSet);
		//ShapeApprox->Generate_ConvexHulls(*ShapePrimitiveSet);
		//ShapeApprox->Generate_ProjectedHulls(*ShapePrimitiveSet, UE::Geometry::FMeshSimpleShapeApproximation::EProjectedHullAxisMode::SmallestVolume);
		ShapePrimitiveSet->RemoveContainedGeometry();
		IsApproximated = true;
	}
	else
	{
		FString UniversalString = "Call AMyActor::Initialize(..) before AMyActor::Approximate";
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, UniversalString);
	}
}

void AMyActor::DrawPrimitives()
{
	if (IsApproximated)
	{		
		if (Verbose)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NumPrimitiveBoxes = " + FString::FromInt(ShapePrimitiveSet->Boxes.Num()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NumPrimitiveCapsules = " + FString::FromInt(ShapePrimitiveSet->Capsules.Num()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NumPrimitiveSpheres = " + FString::FromInt(ShapePrimitiveSet->Spheres.Num()));
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "NumPrimitiveConvexes = " + FString::FromInt(ShapePrimitiveSet->Convexes.Num()));
		}
		DrawBoxes();
		
		DrawCapsules();

		DrawSpheres();

		DrawConvexes();
	}
}

void AMyActor::PrintDynMeshProperties(const UE::Geometry::FDynamicMesh3 &DynMesh, FString OutputStr)
{
	if (DynMesh.CheckValidity())
	{
		OutputStr.Append("CheckValidity = true; VertexCount = ");
	}
	else
	{
		OutputStr.Append("CheckValidity = false; VertexCount = ");
	}
	OutputStr.Append(FString::FromInt(DynMesh.VertexCount()));
	OutputStr.Append("; TriangleCount = ");
	OutputStr.Append(FString::FromInt(DynMesh.TriangleCount()));
	OutputStr.Append("; EdgeCount = ");
	OutputStr.Append(FString::FromInt(DynMesh.EdgeCount()));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, OutputStr);
}

void AMyActor::PrintDynMeshProperties(const TArray<UE::Geometry::FDynamicMesh3>& DynMeshArr)
{
	FString OutputStr = "";
	OutputStr.Append("DynamicMeshArr contains ");
	OutputStr.Append(FString::FromInt(DynMeshArr.Num()));
	OutputStr.Append(" Element(s).");
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, OutputStr);
	for (int32 i = 0; i < DynamicSourceMeshesObjArr.Num(); i++)
	{
		OutputStr = "Element ";
		OutputStr.Append(FString::FromInt(i));
		OutputStr.Append(": ");
		PrintDynMeshProperties(DynMeshArr[i], OutputStr);
	}
}

void AMyActor::PrintApproximatedSizes(const int32& index, const UE::Geometry::EDetectedSimpleShapeType Type, FString OutputStr)
{
	FColor Color = FColor::Blue;
	FVector Corner0;

	switch (Type)
	{
	case UE::Geometry::EDetectedSimpleShapeType::Sphere:
		OutputStr.Append("App. is Sphere: Radius = ");
		OutputStr.Append(FString::SanitizeFloat(ShapePrimitiveSet->Spheres[index].Sphere.Radius));
		OutputStr.Append("  Center = ");
		OutputStr.Append(ShapePrimitiveSet->Spheres[index].Sphere.Center.ToString());
		break;
	case UE::Geometry::EDetectedSimpleShapeType::Capsule:
		OutputStr.Append("App. is Capsule: Radius = ");
		OutputStr.Append(FString::SanitizeFloat(ShapePrimitiveSet->Capsules[index].Capsule.Radius));
		OutputStr.Append("  Length = ");
		OutputStr.Append(FString::SanitizeFloat(ShapePrimitiveSet->Capsules[index].Capsule.Length()));
		OutputStr.Append("  Center = ");
		OutputStr.Append(ShapePrimitiveSet->Capsules[index].Capsule.Center().ToString());
		break;
	case UE::Geometry::EDetectedSimpleShapeType::Box:
		Corner0 = ShapePrimitiveSet->Boxes[index].Box.GetCorner(0);
		OutputStr.Append("App. is Box: AxisX = ");
		OutputStr.Append((ShapePrimitiveSet->Boxes[index].Box.GetCorner(1)-Corner0).ToString());
		OutputStr.Append("  AxisY = ");
		OutputStr.Append((ShapePrimitiveSet->Boxes[index].Box.GetCorner(3)-Corner0).ToString());
		OutputStr.Append("  AxisZ = ");
		OutputStr.Append((ShapePrimitiveSet->Boxes[index].Box.GetCorner(4) - Corner0).ToString());
		OutputStr.Append("  Center = ");
		OutputStr.Append(ShapePrimitiveSet->Boxes[index].Box.Center().ToString());
		break;
	default:
		Color = FColor::Red;
		OutputStr = "PrintApproximatedSizes failed";
		break;
	}
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, Color, OutputStr);
	std::ofstream File;
	FString Filename = FPaths::GameSourceDir() + "Primitives.txt";
	File.open(TCHAR_TO_UTF8(*Filename), std::ios_base::app);
	if (File.is_open())
	{
		File << TCHAR_TO_UTF8(*OutputStr);
		File << std::endl;
		File.close();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "PrintApproximatedSizes: Writing into File failed");
	}
}

void AMyActor::DrawInitializedDynMesh()
{
	FTransform RelativeTransform(FRotator::ZeroRotator, FVector3d::ZeroVector, FVector3d::OneVector);
	
	for (int32 i = 0; i < DynamicSourceMeshesObjArr.Num(); i++)
	{
		MakeStaticMeshActorFromDynamicMesh(FName(GetName() + FString("DynamicMesh") + FString::FromInt(i)), &DynamicSourceMeshesObjArr[i], RelativeTransform);
	}
}

AStaticMeshActor* AMyActor::MakeStaticMeshActorFromDynamicMesh(const FName Name,const UE::Geometry::FDynamicMesh3* DynMesh, const FTransform& RelativeTrans)
{
	UStaticMesh* NewStaticMesh;
	FActorSpawnParameters Pars;
	Pars.Owner = this;
	Pars.Name = Name;
	FTransform Transform = FTransform::Identity; //this->GetActorTransform() * RelativeTrans;
	Transform.SetTranslation(this->GetActorTransform().TransformPosition(RelativeTrans.GetTranslation()));
	Transform.SetRotation( this->GetActorTransform().TransformRotation(RelativeTrans.Rotator().Quaternion()));
	AStaticMeshActor* NewMeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Transform, Pars);
	NewMeshActor->FinishSpawning(Transform);
	NewMeshActor->SetMobility(EComponentMobility::Type::Stationary);
	NewMeshActor->SetActorScale3D(GetActorScale3D());
	OwnedStaticMeshActors.Add(NewMeshActor);

	NewStaticMesh = NewObject<UStaticMesh>(NewMeshActor, Name );
	StaticPrimitives.Add(NewStaticMesh);

	FMeshDescription NewMeshDescription;
	FStaticMeshAttributes NewStaticMeshAttributes(NewMeshDescription);
	NewStaticMeshAttributes.Register();
	Converter.Convert(DynMesh, NewMeshDescription);

	TArray<const FMeshDescription*> NewMeshDescriptionPtrs;
	NewMeshDescriptionPtrs.Emplace(&NewMeshDescription);
	NewStaticMesh->BuildFromMeshDescriptions(NewMeshDescriptionPtrs);	
	
	NewMeshActor->GetStaticMeshComponent()->SetStaticMesh(NewStaticMesh);
	
	//MaterialInterface = UMaterial::GetDefaultMaterial(MD_Surface);
	NewMeshActor->GetStaticMeshComponent()->SetMaterial(0, UMaterialInstanceDynamic::Create(MaterialInterface, NewMeshActor->GetStaticMeshComponent() ));
	

	if (Verbose)
	{
		DrawDebugCoordinateSystem(GetWorld(), this->GetActorLocation(), this->GetActorRotation(), 200.0f, true, 10.0f, (uint8)0U, 5.0f);
	}

	return NewMeshActor;
}


void AMyActor::DrawBoxes()
{
	UE::Geometry::FGridBoxMeshGenerator Generator;
	
	for (int32 i = 0; i < ShapePrimitiveSet->Boxes.Num(); i++)
	{
		Generator.Box = ShapePrimitiveSet->Boxes[i].Box;
		UE::Geometry::FDynamicMesh3 DynamicPrimitive(&Generator.Generate());
		FTransform RelativeTransform = FTransform::Identity;
		MakeStaticMeshActorFromDynamicMesh(FName(GetName() + FString("PrimitiveBox") + FString::FromInt(i)), &DynamicPrimitive, RelativeTransform);
	
		PrintApproximatedSizes(i, UE::Geometry::EDetectedSimpleShapeType::Box);
	}
}

void AMyActor::DrawCapsules()
{
	UE::Geometry::FCapsuleGenerator Generator;
	Generator.NumCircleSteps = 50;
	Generator.NumHemisphereArcSteps = 50;
	for (int32 i = 0; i < ShapePrimitiveSet->Capsules.Num(); i++)
	{
		Generator.Radius = ShapePrimitiveSet->Capsules[i].Capsule.Radius;
		Generator.SegmentLength = ShapePrimitiveSet->Capsules[i].Capsule.Length();
		UE::Geometry::FDynamicMesh3 DynamicPrimitive(&Generator.Generate());
	
		FVector3d Vertex; // Set coordinate system of the Capsule to its center
		for (int32 v = 0; v < DynamicPrimitive.VertexCount(); v++)
		{
			Vertex = DynamicPrimitive.GetVertexRef(v);
			DynamicPrimitive.SetVertex(v, Vertex - FVector3d::ZAxisVector* ShapePrimitiveSet->Capsules[i].Capsule.Length()/2.0f);
		}
		FVector3d Position = ShapePrimitiveSet->Capsules[i].Capsule.Center() + (ShapePrimitiveSet->Capsules[i].Capsule.Length()/2.0f) * ShapePrimitiveSet->Capsules[i].Capsule.Direction();
		FQuat4d Rot;
		Rot = Rot.FindBetweenVectors(ShapePrimitiveSet->Capsules[i].Capsule.Direction(), -FVector3d::ZAxisVector);
		FTransform RelativeTransform(Rot, ShapePrimitiveSet->Capsules[i].Capsule.Center(), FVector3d::OneVector);

		MakeStaticMeshActorFromDynamicMesh(FName(GetName() + FString("PrimitiveCapsule") + FString::FromInt(i)), &DynamicPrimitive,RelativeTransform);
		
		PrintApproximatedSizes(i, UE::Geometry::EDetectedSimpleShapeType::Capsule);
	}
}

void AMyActor::DrawSpheres()
{
	UE::Geometry::FSphereGenerator Generator;
	Generator.NumPhi = 50;
	Generator.NumTheta = 50;
	for (int32 i = 0; i < ShapePrimitiveSet->Spheres.Num(); i++)
	{
		Generator.Radius = ShapePrimitiveSet->Spheres[i].Sphere.Radius;
		UE::Geometry::FDynamicMesh3 DynamicPrimitive(&Generator.Generate());
		FTransform RelativeTransform(FRotator::ZeroRotator, ShapePrimitiveSet->Spheres[i].Sphere.Center, FVector3d::OneVector);
		MakeStaticMeshActorFromDynamicMesh(FName(GetName() + FString("PrimitiveSphere") + FString::FromInt(i)), &DynamicPrimitive, RelativeTransform);
	
		PrintApproximatedSizes(i, UE::Geometry::EDetectedSimpleShapeType::Sphere);
	}
}

void AMyActor::DrawConvexes()
{
	for (int32 i = 0; i < ShapePrimitiveSet->Convexes.Num(); i++)
	{
		FTransform RelativeTransform = FTransform::Identity;
		UE::Geometry::FDynamicMesh3 DynamicPrimitive = ShapePrimitiveSet->Convexes[i].Mesh;
		MakeStaticMeshActorFromDynamicMesh(FName(GetName() + FString("PrimitiveConvex") + FString::FromInt(i)), &DynamicPrimitive, RelativeTransform);
	}
}

void AMyActor::Normalize(const TArray<FVector3d>& Scaling)
{
	FVector3d ThisVertex;
	for (int32 i = 0; i < DynamicSourceMeshesObjArr.Num(); i++)
	{
		if (Scaling[i] != FVector3d::One())
		{
			for (int32 VertexID = 0; VertexID < DynamicSourceMeshesObjArr[i].VertexCount(); VertexID++)
			{
				ThisVertex = DynamicSourceMeshesObjArr[i].GetVertex(VertexID);
				ThisVertex *= Scaling[i];
				DynamicSourceMeshesObjArr[i].SetVertex(VertexID, ThisVertex);
			}
		}
	}
	
}

void AMyActor::RelatviveTransform(const TArray<FTransform>& Transform)
{
	FVector3d ThisVertex;
	for (int32 i = 0; i < DynamicSourceMeshesObjArr.Num(); i++)
	{
		if (Transform[i].GetLocation() != FVector3d::One() || Transform[i].GetRotation() != FQuat4d::Identity)
		{
			for (int32 VertexID = 0; VertexID < DynamicSourceMeshesObjArr[i].VertexCount(); VertexID++)
			{
				ThisVertex = DynamicSourceMeshesObjArr[i].GetVertex(VertexID);
				ThisVertex = Transform[i].TransformPosition(ThisVertex);
				DynamicSourceMeshesObjArr[i].SetVertex(VertexID, ThisVertex);
			}
		}
	}
}


// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

