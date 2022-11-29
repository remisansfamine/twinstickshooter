// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
#include "TwinStickShooterGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStateEvent, APlayerState*, BestPlayer);

/**
 * 
 */
UCLASS()
class TWINSTICKSHOOTER_API ATwinStickShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPlayerStateEvent OnBestPlayerChange;

	UPROPERTY(BlueprintAssignable)
	FPlayerStateEvent OnWinReached;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_BestPlayer, BlueprintReadOnly)
	APlayerState* BestPlayer = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Winner, BlueprintReadOnly)
	APlayerState* Winner = nullptr;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	float ScoreToReach = 20.f;

	UFUNCTION()
	void OnRep_BestPlayer() const;

	UFUNCTION()
	void OnRep_Winner() const;

	void UpdateBestScore(APlayerState* PlayerState);
};
