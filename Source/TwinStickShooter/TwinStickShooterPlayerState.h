// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TwinStickShooterPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreEvent, float, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUsernameEvent, FString, Username);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FColorEvent, FLinearColor, LinearColor);

/**
 * 
 */
UCLASS()
class TWINSTICKSHOOTER_API ATwinStickShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FScoreEvent OnScoreChange;
	
	UPROPERTY(BlueprintAssignable)
	FUsernameEvent OnUsernameChange;

	UPROPERTY(BlueprintAssignable)
	FColorEvent OnColorChange;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int DeathCount = 0;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	int KillCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Username, EditAnywhere, BlueprintReadOnly)
	FString Username = "Player";

	UPROPERTY(ReplicatedUsing = OnRep_Color, EditAnywhere, BlueprintReadOnly)
	FLinearColor Color = FLinearColor::Red;
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void SetServerUsername(const FString& NewUsername);
	void SetUsername(const FString& NewUsername);

	UFUNCTION()
	void OnRep_Username() const;

	UFUNCTION()
	void OnRep_Color() const;

	virtual void OnRep_Score() override;

	UFUNCTION(Server, Reliable)
	void SetServerColor(const FLinearColor& NewColor);
	void SetColor(const FLinearColor& NewColor);

	void UpdateData();

	UFUNCTION(Server, Reliable)
	void UpdateGMScore();
};
