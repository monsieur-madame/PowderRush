#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PowderTypes.h"
#include "PowderTuningProfile.generated.h"

UCLASS(BlueprintType)
class POWDERRUSH_API UPowderTuningProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	FMovementTuning Movement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	FCameraTuning Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning", meta = (ClampMin = "0.0"))
	float BlendTime = 1.0f;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PowderTuningProfile", GetFName());
	}
};
