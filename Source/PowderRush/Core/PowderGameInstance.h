// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Meta/PowderSaveGame.h"
#include "PowderGameInstance.generated.h"

struct FRunStats;

UCLASS()
class POWDERRUSH_API UPowderGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintPure, Category = "PowderRush")
	int32 GetTotalCoins() const { return TotalCoins; }

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void AddCoins(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "PowderRush")
	int32 GetGems() const { return Gems; }

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void AddGems(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "PowderRush")
	int32 GetHighScore() const { return HighScore; }

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void TrySetHighScore(int32 Score);

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void OnRunCompleted(const FRunStats& RunStats);

	UFUNCTION(BlueprintPure, Category = "PowderRush")
	const FLifetimeStats& GetLifetimeStats() const { return LifetimeStats; }

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void SaveGame();

	UFUNCTION(BlueprintCallable, Category = "PowderRush")
	void LoadGame();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 TotalCoins = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 Gems = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 HighScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	FLifetimeStats LifetimeStats;
};
