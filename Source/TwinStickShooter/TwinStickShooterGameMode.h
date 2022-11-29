// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/GameMode.h"
#include "TwinStickShooterGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStateEvent, APlayerState*, BestPlayer);

UCLASS(minimalapi)
class ATwinStickShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	APlayerState* BestPlayer = nullptr;
	
	UPROPERTY(BlueprintAssignable)
	FPlayerStateEvent OnBestPlayerChange;

	UPROPERTY(BlueprintAssignable)
	FPlayerStateEvent OnWinReached;

	UPROPERTY(Replicated, BlueprintReadWrite)
	float ScoreToReach = 20.f;
	
	ATwinStickShooterGameMode();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void UpdateBestScore(APlayerState* PlayerState);
};



