#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PowderTypes.h"
#include "PowderSurfaceProfile.generated.h"

UCLASS(BlueprintType)
class POWDERRUSH_API UPowderSurfaceProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	FSurfaceProperties Surface;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PowderSurfaceProfile", GetFName());
	}
};
