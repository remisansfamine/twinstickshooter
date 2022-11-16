// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "TwinStickShooterPawn.h"
#include "TwinStickShooter.h"
#include "TwinStickShooterProjectile.h"
#include "TimerManager.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White,text)

const FName ATwinStickShooterPawn::MoveForwardBinding("MoveForward");
const FName ATwinStickShooterPawn::MoveRightBinding("MoveRight");
const FName ATwinStickShooterPawn::FireForwardBinding("FireForward");
const FName ATwinStickShooterPawn::FireRightBinding("FireRight");

ATwinStickShooterPawn::ATwinStickShooterPawn()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetAbsolute(false, true, false); // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
	bIsAlive = true;

	CurrentHealthPoints = MaxHealthPoints;

	SetReplicates(true);
}

void ATwinStickShooterPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
	PlayerInputComponent->BindAxis(FireRightBinding);
}

void ATwinStickShooterPawn::Tick(float DeltaSeconds)
{
	// Cancel movement logic for server
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy)
		return;
	
	if (!bIsAlive)
		return;

	// Find movement direction
	ForwardValue = GetInputAxisValue(MoveForwardBinding);
	RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f);

	// Try and fire a shot
	FireShot(FireDirection);

	// Apply movement
	ComputeMove(DeltaSeconds);
}

void ATwinStickShooterPawn::ComputeMove(float DeltaSeconds)
{
	if (ForwardValue == 0.0f && RightValue == 0.0f)
		return;

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	Move(Movement);
	ServerMove(Movement);
}

void ATwinStickShooterPawn::Move(const FVector& Movement)
{
	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}
}

void ATwinStickShooterPawn::FireShot(FVector FireDirection)
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<ATwinStickShooterProjectile>(SpawnLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ATwinStickShooterPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void ATwinStickShooterPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void ATwinStickShooterPawn::Revive()
{
	print("Reviving Player !");
	bIsAlive = true;
	CurrentHealthPoints = MaxHealthPoints;
	ShipMeshComponent->SetVisibility(true);
	ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

float ATwinStickShooterPawn::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f && bIsAlive)
	{
		CurrentHealthPoints -= ActualDamage;
		if (CurrentHealthPoints <= 0)
		{
			print("Player is dead");
			bIsAlive = false;
			CurrentHealthPoints = 0;
			ShipMeshComponent->SetVisibility(false);
			ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//K2_DestroyActor();
			// "revive" player after a delay
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Revive, this, &ATwinStickShooterPawn::Revive, ReviveDelay, false);
		}
	}
	return ActualDamage;
}

void ATwinStickShooterPawn::ServerMove_Implementation(const FVector& Delta)
{
	// Move the client
	Move(Delta);
}

bool ATwinStickShooterPawn::ServerMove_Validate(const FVector& Delta)
{
	return true;
}