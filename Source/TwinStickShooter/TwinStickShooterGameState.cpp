// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinStickShooterGameState.h"
#include "Net/UnrealNetwork.h"

#include "TwinStickShooterInstance.h"

void ATwinStickShooterGameState::BeginPlay()
{
	Super::BeginPlay();
	
	if (UTwinStickShooterInstance* instance = Cast<UTwinStickShooterInstance>(GetGameInstance()))
		ScoreToReach = instance->ScoreToReach;
}


void ATwinStickShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATwinStickShooterGameState, BestPlayer);
	DOREPLIFETIME(ATwinStickShooterGameState, Winner);
	DOREPLIFETIME(ATwinStickShooterGameState, ScoreToReach);
}

void ATwinStickShooterGameState::OnRep_BestPlayer() const
{
	if (OnBestPlayerChange.IsBound())
		OnBestPlayerChange.Broadcast(BestPlayer);
}

void ATwinStickShooterGameState::OnRep_Winner() const
{
	if (OnWinReached.IsBound())
		OnWinReached.Broadcast(Winner);
}

void ATwinStickShooterGameState::UpdateBestScore(APlayerState* PlayerState)
{
	const float PlayerScore = PlayerState->GetScore();
	if (!BestPlayer || PlayerScore > BestPlayer->GetScore())
	{
		BestPlayer = PlayerState;
		OnRep_BestPlayer();
	}

	if (PlayerScore >= ScoreToReach)
	{
		Winner = PlayerState;
		OnRep_Winner();
	}
}
