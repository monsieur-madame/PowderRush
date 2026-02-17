#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PowderSaveGame.generated.h"

USTRUCT(BlueprintType)
struct POWDERRUSH_API FLifetimeStats
{
	GENERATED_BODY()

	// Best-of (updated per run via FMath::Max)
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 TotalRuns = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 BestScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 BestComboChain = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 MostTricksInRun = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 MostNearMissesInRun = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 MostPowerupsInRun = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	float BestDistance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	float HighestMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	float LongestAirTime = 0.0f;

	// Cumulative (added per run)
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 TotalCoinsEarned = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 TotalTricksLanded = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 TotalNearMisses = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	int32 TotalGatesPassed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Stats")
	float TotalDistanceTraveled = 0.0f;
};

UCLASS()
class POWDERRUSH_API UPowderSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "PowderRush|Save")
	int32 TotalCoins = 0;

	UPROPERTY(BlueprintReadWrite, Category = "PowderRush|Save")
	int32 Gems = 0;

	UPROPERTY(BlueprintReadWrite, Category = "PowderRush|Save")
	int32 HighScore = 0;

	UPROPERTY(BlueprintReadWrite, Category = "PowderRush|Save")
	FLifetimeStats LifetimeStats;

	UPROPERTY(BlueprintReadWrite, Category = "PowderRush|Save")
	int32 SaveVersion = 1;

	static const FString SaveSlotName;
	static constexpr int32 SaveUserIndex = 0;
};
