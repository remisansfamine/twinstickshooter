// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinStickShooterPlayerState.h"

#include "GameFramework/GameSession.h"
#include "Net/UnrealNetwork.h"

#include "TwinStickShooterInstance.h"

void ATwinStickShooterPlayerState::BeginPlay()
{
	UpdateUsername();
}

void ATwinStickShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATwinStickShooterPlayerState, DeathCount);
	DOREPLIFETIME(ATwinStickShooterPlayerState, KillCount);
	DOREPLIFETIME(ATwinStickShooterPlayerState, Username);
}

void ATwinStickShooterPlayerState::UpdateUsername()
{
	if (GetLocalRole() != ENetRole::ROLE_AutonomousProxy || GetRemoteRole() == ENetRole::ROLE_Authority)
	{
		if (UTwinStickShooterInstance* instance = Cast<UTwinStickShooterInstance>(GetGameInstance()))
			SetUsername(instance->Username);
	}
}

void ATwinStickShooterPlayerState::SetUsername_Implementation(const FString& NewUsername)
{
	Username = NewUsername;
}
