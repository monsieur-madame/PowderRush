#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PowderTypes.h"
#include "PowderWeatherProfile.generated.h"

/**
 * Data asset wrapping FWeatherConfig for editor-driven weather presets.
 */
UCLASS(BlueprintType)
class POWDERRUSH_API UPowderWeatherProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	FWeatherConfig Config;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("PowderWeatherProfile"), GetFName());
	}
};
