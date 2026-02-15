// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameInstance.h"

void UPowderGameInstance::Init()
{
	Super::Init();

	// TODO: Load saved data
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
