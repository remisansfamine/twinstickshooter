// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Character.h"
#include "TwinStickShooterPawn.generated.h"

USTRUCT()
struct FNetworkMove
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector DeltaMovement;

	UPROPERTY()
	float Timestamp;
};


UCLASS(Blueprintable)
class ATwinStickShooterPawn : public APawn
{
	GENERATED_BODY()

	const float MaxSqrdLocationDistanceDiff = 100.f;

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	ATwinStickShooterPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;
	
	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	int MaxHealthPoints = 10;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float ReviveDelay = 2.0f;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

private:
	float TimeStamp = 0.f;
	
	UFUNCTION(Client, Reliable)
    void ClientAdjustMovement(const FVector& ClientLocation, const FVector& ClientVelocity, float ServerTimeStamp);

	UFUNCTION(Server, Reliable)
	void ServerSpawnProjectile(const FVector& FireDirection);

	UFUNCTION(NetMulticast, Reliable)
	void MultiSpawnProjectile(const FVector& FireDirection);
	
	UFUNCTION(Server, Unreliable, WithValidation)
    void ServerMove(const FVector& Delta, const FVector& ClientLocation);
	void ServerMoveHandleClientError(const FVector& ClientLocation);
	
	void ReplicateMoveToServer(const FVector& Delta);
	void Move(const FVector& Delta) const;

	TQueue<FNetworkMove> SavedMoves;
	
	void ClientUpdatePositionAfterServerUpdate();
	
	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/* Flag to set death state */
	uint32 bIsAlive : 1;

	bool bUpdatePosition = false;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	FTimerHandle TimerHandle_Revive;

	UPROPERTY()
	float ForwardValue;
	UPROPERTY()
	float RightValue;

	UPROPERTY()
	FVector MoveDirection;

	int CurrentHealthPoints;

	/* Fire a shot in the specified direction */
	void FireShot(const FVector& FireDirection);

	void ServerShot(const FVector& FireDirection, const FVector& ClientLocation, bool bIsVirtualShot);

	/* Compute Pawn movement */
	void ComputeMove(float DeltaSeconds);

	/* Revive Pawn */
	UFUNCTION(NetMulticast, Reliable)
	void MultiRevive();
	
	/* Kill pawn Pawn */
	void ServerDie(AActor* FinalDamager);
	
	UFUNCTION(NetMulticast, Reliable)
	void MultiDie();

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};

