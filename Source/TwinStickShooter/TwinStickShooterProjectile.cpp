// Copyright 1998-2015 Epic Games, Inc. All Rights Reserve

#include "TwinStickShooterProjectile.h"
#include "TwinStickShooter.h"
#include "TwinStickShooterPawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White,text)

ATwinStickShooterProjectile::ATwinStickShooterProjectile() 
{
	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("/Game/TwinStick/Meshes/TwinStickProjectile.TwinStickProjectile"));

	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ATwinStickShooterProjectile::OnHit);		// set up a notification for when this component hits something
	RootComponent = ProjectileMesh;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void ATwinStickShooterProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and damages if it is the server projectile
	// Only add impulse and destroy projectile if we hit a physics
	if (!bIsVirtual && OtherActor && OtherActor != this)
	{
		if (OtherActor->GetClass()->IsChildOf(ATwinStickShooterPawn::StaticClass()))
		{
			//print(TEXT("hit ennemy"));
			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);

			OtherActor->TakeDamage(DamageAmount, DamageEvent, nullptr, this);
		}
		else if ((OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
		{
			//print(TEXT("add impulse"));
			OtherComp->AddImpulseAtLocation(GetVelocity() * 20.0f, GetActorLocation());
		}
	}

	Destroy();
}