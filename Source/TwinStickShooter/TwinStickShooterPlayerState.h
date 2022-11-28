// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TwinStickShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TWINSTICKSHOOTER_API ATwinStickShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int DeathCount = 0;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int KillCount = 0;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	FString Username = "Player";

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void SetUsername(const FString& NewUsername);

	void UpdateUsername();
};
