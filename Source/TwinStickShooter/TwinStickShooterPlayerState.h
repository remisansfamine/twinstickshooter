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

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	FString Username = "Player";

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	FLinearColor Color = FLinearColor::Red;
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void SetServerUsername(const FString& NewUsername);
	void SetUsername(const FString& NewUsername);

	UFUNCTION(Server, Reliable)
	void SetServerColor(const FLinearColor& NewColor);
	void SetColor(const FLinearColor& NewColor);

	void UpdateData();


};
