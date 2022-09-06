// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AFPSProjectile::AFPSProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;
	
	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

}


void AFPSProjectile::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AFPSProjectile::Explode, 3.0f, false);
}

void AFPSProjectile::Explode()
{
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation(), FRotator::ZeroRotator, FVector(5.0f));

	// Allow BP to trigger additional logic
	BlueprintExplode();

	Destroy();
}


void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics object
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
	{
		/*float RandomIntensity = FMath::RandRange(200.0f, 500.0f);
		OtherComp->AddImpulseAtLocation(GetVelocity() * RandomIntensity, GetActorLocation());
		*/
		
		UE::Geometry::FDynamicMesh3 SourceDynMesh;
			
		int32 LODidx = 0;
		FMeshDescription* SourceMeshDescPtr = nullptr;
		FMeshDescriptionToDynamicMesh Converter;
		TArray<UE::Geometry::FDynamicMesh3> SourceDynMeshArr;
		TArray<FTransform> RelativeTransArr;
		TArray<UStaticMeshComponent*> OtherActorMeshComponents;
		FTransform RelativeTransform = OtherActor->GetActorTransform();
		FTransform OtherActorTransform = OtherActor->GetActorTransform();
		FVector OtherActorScale = OtherActor->GetActorScale();
		OtherActor->GetComponents<UStaticMeshComponent>(OtherActorMeshComponents);
		FString UniversalString = "The Actor ";
		OtherActor->AppendName(UniversalString);
		UniversalString.Append(" has ");
		UniversalString.Append(FString::FromInt(OtherActorMeshComponents.Num()) );
		UniversalString.Append(" Mesh Component(s)");
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, UniversalString);
		for (int32 i = 0; i < OtherActorMeshComponents.Num(); i++)
		{

			UStaticMeshComponent* StaticMeshComponent = OtherActorMeshComponents[i];
			UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			if (StaticMesh->IsMeshDescriptionValid(LODidx))
			{
				SourceMeshDescPtr = StaticMesh->GetMeshDescription(LODidx);
				SourceDynMesh.Clear();
				Converter.Convert(SourceMeshDescPtr, SourceDynMesh);
				SourceDynMeshArr.Add(SourceDynMesh);
				RelativeTransform = StaticMeshComponent->GetComponentTransform().GetRelativeTransform(OtherActorTransform);
				RelativeTransform.SetLocation(RelativeTransform.GetLocation() * OtherActorScale);
				RelativeTransform.SetScale3D(StaticMeshComponent->GetComponentScale());
				RelativeTransArr.Add(RelativeTransform);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "StaticMesh number " + FString::FromInt(i) + " has no valid Mesh Description.");
			}
		}
		
		OtherActorTransform.SetScale3D(FVector::One());
		AMyActor* DynMeshActor = GetWorld()->SpawnActor<AMyActor>(AMyActor::StaticClass(), OtherActorTransform);
		DynMeshActor->FinishSpawning(OtherActorTransform);
		DynMeshActor->Initialize(SourceDynMeshArr, RelativeTransArr);
		DynMeshActor->Approximate();
		DynMeshActor->DrawPrimitives();
		//DynMeshActor->DrawInitializedDynMesh();
		
		OtherActor->SetLifeSpan(1.0f);

		UMaterialInstanceDynamic* MatInst = OtherComp->CreateDynamicMaterialInstance(0);
		if (MatInst)
		{
			FLinearColor NewColor = FLinearColor::Red;
			MatInst->SetVectorParameterValue("Color", NewColor);
		}
		
		Explode();
	}
}