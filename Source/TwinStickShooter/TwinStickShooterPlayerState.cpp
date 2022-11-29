// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinStickShooterPlayerState.h"

#include "TwinStickShooterGameState.h"
#include "GameFramework/GameSession.h"
#include "Net/UnrealNetwork.h"

#include "TwinStickShooterInstance.h"

void ATwinStickShooterPlayerState::BeginPlay()
{
	UpdateData();
}

void ATwinStickShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATwinStickShooterPlayerState, DeathCount);
	DOREPLIFETIME(ATwinStickShooterPlayerState, KillCount);
	DOREPLIFETIME(ATwinStickShooterPlayerState, Username);
	DOREPLIFETIME(ATwinStickShooterPlayerState, Color);
}

void ATwinStickShooterPlayerState::UpdateData()
{
	if (UTwinStickShooterInstance* instance = Cast<UTwinStickShooterInstance>(GetGameInstance()))
	{
		SetUsername(instance->PlayerUsername);
		SetColor(instance->PlayerColor);
	}
}

void ATwinStickShooterPlayerState::SetUsername(const FString& NewUsername)
{
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy || GetRemoteRole() == ENetRole::ROLE_Authority)
		SetServerUsername(NewUsername);
}

void ATwinStickShooterPlayerState::SetServerUsername_Implementation(const FString& NewUsername)
{
	Username = NewUsername;

	if (OnUsernameChange.IsBound())
		OnUsernameChange.Broadcast(Username);
}

void ATwinStickShooterPlayerState::OnRep_Username() const
{
	if (OnUsernameChange.IsBound())
		OnUsernameChange.Broadcast(Username);
}

void ATwinStickShooterPlayerState::SetColor(const FLinearColor& NewColor)
{
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy || GetRemoteRole() == ENetRole::ROLE_Authority)
		SetServerColor(NewColor);
}

void ATwinStickShooterPlayerState::SetServerColor_Implementation(const FLinearColor& NewColor)
{
	Color = NewColor;

	if (OnColorChange.IsBound())
		OnColorChange.Broadcast(Color);
}

void ATwinStickShooterPlayerState::OnRep_Color() const
{
	if (OnColorChange.IsBound())
		OnColorChange.Broadcast(Color);
}

void ATwinStickShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	if (OnScoreChange.IsBound())
		OnScoreChange.Broadcast(GetScore());

	UpdateGMScore();
}

void ATwinStickShooterPlayerState::UpdateGMScore_Implementation()
{
	if (ATwinStickShooterGameState* GS = Cast<ATwinStickShooterGameState>(GetWorld()->GetGameState()))
		GS->UpdateBestScore(this);
}
