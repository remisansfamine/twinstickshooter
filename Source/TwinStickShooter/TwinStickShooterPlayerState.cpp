// Fill out your copyright notice in the Description page of Project Settings.


#include "TwinStickShooterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ATwinStickShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATwinStickShooterPlayerState, DeathCount);
	DOREPLIFETIME(ATwinStickShooterPlayerState, KillCount);
}
