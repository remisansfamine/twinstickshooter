// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinStickShooterPlayerState.h"

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
}

void ATwinStickShooterPlayerState::UpdateData()
{
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy || GetRemoteRole() == ENetRole::ROLE_Authority)
	{
		if (UTwinStickShooterInstance* instance = Cast<UTwinStickShooterInstance>(GetGameInstance()))
		{
			SetUsername(instance->PlayerUsername);
			SetColor(instance->PlayerColor);
		}
	}
}

void ATwinStickShooterPlayerState::SetUsername(const FString& NewUsername)
{
	Username = NewUsername;
	SetServerUsername(NewUsername);
}

void ATwinStickShooterPlayerState::SetServerUsername_Implementation(const FString& NewUsername)
{
	Username = NewUsername;
}

void ATwinStickShooterPlayerState::SetColor(const FLinearColor& NewColor)
{
	Color = NewColor;
	SetServerColor(NewColor);
}

void ATwinStickShooterPlayerState::SetServerColor_Implementation(const FLinearColor& NewColor)
{
	Color = NewColor;
}