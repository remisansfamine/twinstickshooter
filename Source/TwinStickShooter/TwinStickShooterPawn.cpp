// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "TwinStickShooterPawn.h"
#include "TwinStickShooter.h"
#include "TwinStickShooterProjectile.h"
#include "TimerManager.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>

#include "TwinStickShooterPlayerState.h"

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
	Super::Tick(DeltaSeconds);
	
	if (!bIsAlive)
		return;

	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	const FVector FireDirection = FVector(FireForwardValue, FireRightValue, 0.f).GetSafeNormal();

	// Find movement direction
	ForwardValue = GetInputAxisValue(MoveForwardBinding);
	RightValue = GetInputAxisValue(MoveRightBinding);
	
	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Apply movement and shots
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FireShotOnServer(FireDirection);
		ComputeMoveOnServer(DeltaSeconds);
	}
	else if (GetLocalRole() == ROLE_Authority && GetNetMode() == ENetMode::NM_ListenServer)
	{
		FireShotOnHost(FireDirection);
		ComputeMoveOnHost(DeltaSeconds);
	}
}

void ATwinStickShooterPawn::ComputeMoveOnServer(float DeltaSeconds)
{
	if (ForwardValue == 0.0f && RightValue == 0.0f)
		return;

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// Handle Server movement response
	ClientUpdatePositionAfterServerUpdate();
	
	ReplicateMoveToServer(Movement);
}

void ATwinStickShooterPawn::ComputeMoveOnHost(float DeltaSeconds)
{
	if (ForwardValue == 0.0f && RightValue == 0.0f)
		return;

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	Move(Movement);
}

void ATwinStickShooterPawn::ReplicateMoveToServer(const FVector& Delta)
{
	const FNetworkMove SavedMove = FNetworkMove { Delta, TimeStamp };
	SavedMoves.Enqueue(SavedMove);

	Move(Delta);
	ServerMove(Delta, GetActorLocation());
}

void ATwinStickShooterPawn::Move(const FVector& Delta) const
{
	// If non-zero size, move this actor
	if (Delta.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Delta.Rotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Delta, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Delta, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}
}


void ATwinStickShooterPawn::ClientUpdatePositionAfterServerUpdate()
{
	if (!bUpdatePosition)
		return;

	FNetworkMove Movement;
	while (SavedMoves.Dequeue(Movement))
	{
		if (Movement.Timestamp >= TimeStamp)
			Move(Movement.DeltaMovement);
	}
	
	bUpdatePosition = false;
}

void ATwinStickShooterPawn::ServerShot(const FVector& FireDirection, const FVector& ClientLocation, bool bIsVirtualShot)
{
	const FRotator FireRotation = FireDirection.Rotation();
	// Spawn projectile at an offset from this pawn
	const FVector SpawnLocation = ClientLocation + FireRotation.RotateVector(GunOffset);

	UWorld* const World = GetWorld();
	if (World)
	{
		// Spawn the projectile
		if (ATwinStickShooterProjectile* projectile = World->SpawnActor<ATwinStickShooterProjectile>(SpawnLocation, FireRotation))
		{
			projectile->SetOwner(this);
			projectile->bIsVirtual = bIsVirtualShot;
		}
	}

	bCanFire = false;
	World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ATwinStickShooterPawn::ShotTimerExpired, FireRate);

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void ATwinStickShooterPawn::FireShotOnServer(const FVector& FireDirection)
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			ServerSpawnProjectile(FireDirection);
		}
	}
}

void ATwinStickShooterPawn::FireShotOnHost(const FVector& FireDirection)
{
	// If we it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			MultiSpawnProjectile(FireDirection.GetSafeNormal());
		}
	}
}

void ATwinStickShooterPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void ATwinStickShooterPawn::ServerDie(AActor* FinalDamager)
{
	if (ATwinStickShooterProjectile* projectile = Cast<ATwinStickShooterProjectile>(FinalDamager))
	{

		ATwinStickShooterPawn* killer = Cast<ATwinStickShooterPawn>(projectile->GetOwner());
		ATwinStickShooterPlayerState* killerPS = Cast<ATwinStickShooterPlayerState>(killer->GetPlayerState());
		killerPS->KillCount++;		
		killerPS->SetScore(killerPS->GetScore() + 1);
	}

	ATwinStickShooterPlayerState* selfPS = Cast<ATwinStickShooterPlayerState>(GetPlayerState());
	selfPS->DeathCount++;
	
	MultiDie();
}

void ATwinStickShooterPawn::MultiDie_Implementation()
{
	bIsAlive = false;
	CurrentHealthPoints = 0;
	ShipMeshComponent->SetVisibility(false);
	ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GetLocalRole() == ENetRole::ROLE_Authority)
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_Revive, this, &ATwinStickShooterPawn::MultiRevive, ReviveDelay, false);
}

void ATwinStickShooterPawn::MultiRevive_Implementation()
{
	bIsAlive = true;
	CurrentHealthPoints = MaxHealthPoints;
	ShipMeshComponent->SetVisibility(true);
	ShipMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

float ATwinStickShooterPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f && bIsAlive)
	{
		CurrentHealthPoints -= ActualDamage;
		if (CurrentHealthPoints <= 0)
			ServerDie(DamageCauser);
	}
	return ActualDamage;
}

void ATwinStickShooterPawn::ServerSpawnProjectile_Implementation(const FVector& FireDirection)
{
	// TODO Use TimeStamp to know where and when the client has shoot
	MultiSpawnProjectile(FireDirection.GetSafeNormal());
}

void ATwinStickShooterPawn::MultiSpawnProjectile_Implementation(const FVector& FireDirection)
{
	ServerShot(FireDirection, GetActorLocation(), GetLocalRole() != ENetRole::ROLE_Authority);
}

void ATwinStickShooterPawn::ClientAdjustMovement_Implementation(const FVector& ClientLocation, const FVector& ClientVelocity, float ServerTimeStamp)
{
	SetActorLocation(ClientLocation);
	RootComponent->ComponentVelocity = ClientVelocity;
	TimeStamp = ServerTimeStamp;
	bUpdatePosition = true;
}

void ATwinStickShooterPawn::ServerMoveHandleClientError(const FVector& ClientLocation)
{
	if (FVector::DistSquared(GetActorLocation(), ClientLocation) > MaxSqrdLocationDistanceDiff)
	{
		// Adjust the client movement
		ClientAdjustMovement(GetActorLocation(), GetRootComponent()->ComponentVelocity, GetWorld()->RealTimeSeconds);
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, "Error");
	}
}

void ATwinStickShooterPawn::ServerMove_Implementation(const FVector& Delta, const FVector& ClientLocation)
{
	// Move the client for every clients
	Move(Delta);

	// Check for a potential position error
	ServerMoveHandleClientError(ClientLocation);
}

bool ATwinStickShooterPawn::ServerMove_Validate(const FVector& Delta, const FVector& ClientLocation)
{
	return true;
}