// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "TwinStickShooterGameMode.h"
#include "TwinStickShooter.h"
#include "TwinStickShooterPawn.h"
#include "TwinStickShooterPlayerState.h"
#include "GameFrameWork/PlayerStart.h"
#include "Net/UnrealNetwork.h"

ATwinStickShooterGameMode::ATwinStickShooterGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = ATwinStickShooterPawn::StaticClass();
	PlayerStateClass = ATwinStickShooterPlayerState::StaticClass();

}

void ATwinStickShooterGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATwinStickShooterGameMode, BestPlayer);
	DOREPLIFETIME(ATwinStickShooterGameMode, ScoreToReach);
}

AActor * ATwinStickShooterGameMode::ChoosePlayerStart_Implementation(AController * Player)
{
	TArray<AActor*> FoundPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), FoundPlayerStarts);

	for (auto& actor : FoundPlayerStarts)
	{
		APlayerStart* playerStart = Cast<APlayerStart>(actor);
		if (playerStart->PlayerStartTag != FName("Taken"))
		{
			playerStart->PlayerStartTag = FName("Taken");
			return playerStart;
		}
	}

	return nullptr;
}

void ATwinStickShooterGameMode::UpdateBestScore(APlayerState* PlayerState)
{
	const float PlayerScore = PlayerState->GetScore();
	if (!BestPlayer || PlayerScore > BestPlayer->GetScore())
	{
		BestPlayer = PlayerState;

		if (OnBestPlayerChange.IsBound())
			OnBestPlayerChange.Broadcast(BestPlayer);
		
		if (PlayerScore >= ScoreToReach)
		{
			if (OnWinReached.IsBound())
				OnWinReached.Broadcast(BestPlayer);
		}
	}
}