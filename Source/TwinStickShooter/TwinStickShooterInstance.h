// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TwinStickShooterInstance.generated.h"

/**
 * 
 */
UCLASS()
class TWINSTICKSHOOTER_API UTwinStickShooterInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerUsername = "Player";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PlayerColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreToReach = 10.f;
};
