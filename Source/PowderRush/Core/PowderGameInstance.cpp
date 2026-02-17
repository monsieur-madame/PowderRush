// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameInstance.h"
#include "Meta/PowderSaveGame.h"
#include "Scoring/ScoreSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UPowderGameInstance::Init()
{
	Super::Init();
	LoadGame();
}

void UPowderGameInstance::AddCoins(int32 Amount)
{
	TotalCoins = FMath::Max(0, TotalCoins + Amount);
}

void UPowderGameInstance::AddGems(int32 Amount)
{
	Gems = FMath::Max(0, Gems + Amount);
}

void UPowderGameInstance::TrySetHighScore(int32 Score)
{
	if (Score > HighScore)
	{
		HighScore = Score;
	}
}

void UPowderGameInstance::OnRunCompleted(const FRunStats& RunStats)
{
	LifetimeStats.TotalRuns++;

	// Best-of per run
	LifetimeStats.BestScore = FMath::Max(LifetimeStats.BestScore, RunStats.TotalScore);
	LifetimeStats.BestComboChain = FMath::Max(LifetimeStats.BestComboChain, RunStats.BestComboChain);
	LifetimeStats.MostTricksInRun = FMath::Max(LifetimeStats.MostTricksInRun, RunStats.TricksLanded);
	LifetimeStats.MostNearMissesInRun = FMath::Max(LifetimeStats.MostNearMissesInRun, RunStats.NearMissCount);
	LifetimeStats.MostPowerupsInRun = FMath::Max(LifetimeStats.MostPowerupsInRun, RunStats.PowerupsCollected);
	LifetimeStats.BestDistance = FMath::Max(LifetimeStats.BestDistance, RunStats.TotalDistance);
	LifetimeStats.HighestMultiplier = FMath::Max(LifetimeStats.HighestMultiplier, RunStats.HighestMultiplier);
	LifetimeStats.LongestAirTime = FMath::Max(LifetimeStats.LongestAirTime, RunStats.LongestAirTime);

	// Cumulative
	LifetimeStats.TotalCoinsEarned += RunStats.CoinsCollected;
	LifetimeStats.TotalTricksLanded += RunStats.TricksLanded;
	LifetimeStats.TotalNearMisses += RunStats.NearMissCount;
	LifetimeStats.TotalGatesPassed += RunStats.GatesPassed;
	LifetimeStats.TotalDistanceTraveled += RunStats.TotalDistance;

	// Update high score and coins
	TrySetHighScore(RunStats.TotalScore);
	AddCoins(RunStats.CoinsCollected);

	SaveGame();
}

void UPowderGameInstance::SaveGame()
{
	UPowderSaveGame* SaveGameObj = Cast<UPowderSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPowderSaveGame::StaticClass()));

	if (!SaveGameObj)
	{
		return;
	}

	SaveGameObj->TotalCoins = TotalCoins;
	SaveGameObj->Gems = Gems;
	SaveGameObj->HighScore = HighScore;
	SaveGameObj->LifetimeStats = LifetimeStats;

	UGameplayStatics::AsyncSaveGameToSlot(SaveGameObj,
		UPowderSaveGame::SaveSlotName, UPowderSaveGame::SaveUserIndex);
}

void UPowderGameInstance::LoadGame()
{
	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(
		UPowderSaveGame::SaveSlotName, UPowderSaveGame::SaveUserIndex);

	if (UPowderSaveGame* SaveGameObj = Cast<UPowderSaveGame>(LoadedSave))
	{
		TotalCoins = SaveGameObj->TotalCoins;
		Gems = SaveGameObj->Gems;
		HighScore = SaveGameObj->HighScore;
		LifetimeStats = SaveGameObj->LifetimeStats;
	}
}
