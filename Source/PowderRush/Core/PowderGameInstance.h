// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PowderGameInstance.generated.h"

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

protected:
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 TotalCoins = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 Gems = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush")
	int32 HighScore = 0;
};
