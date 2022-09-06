// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"
//#include "ShapeApproximation/MeshSimpleShapeApproximation.h"
#include "MeshDescription.h"
#include "UDynamicMesh.h"
#include "MeshDescriptionToDynamicMesh.h"
#include "MyActor.h"
#include "FPSProjectile.generated.h"




class UProjectileMovementComponent;
class USphereComponent;
class UParticleSystem;


UCLASS()
class AFPSProjectile : public AActor
{
	GENERATED_BODY()

protected:

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components")
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	UParticleSystem* ExplosionFX;



	UFUNCTION()
	void Explode();


	//FSimpleShapeSet3 Shape;
	

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintExplode();

public:

	AFPSProjectile();

	virtual void BeginPlay() override;

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }

	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};

