#pragma once

#include "CoreMinimal.h"
#include "Core/PowderTypes.h"
#include "UObject/Interface.h"
#include "PowderSurfaceQueryProvider.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UPowderSurfaceQueryProvider : public UInterface
{
	GENERATED_BODY()
};

class POWDERRUSH_API IPowderSurfaceQueryProvider
{
	GENERATED_BODY()

public:
	/**
	 * Samples terrain surface data nearest to a world position.
	 * @return true when a valid surface was sampled.
	 */
	virtual bool SampleSurfaceAtWorldPosition(
		const FVector& WorldPos,
		FSurfaceProperties& OutSurface,
		float& OutCourseDistance) const = 0;

	/**
	 * Samples the spline-aligned course frame nearest to the given world position.
	 * @return true when a valid course frame was sampled.
	 */
	virtual bool SampleCourseFrameAtWorldPosition(
		const FVector& WorldPos,
		FVector& OutTangent,
		FVector& OutUp,
		float& OutCourseDistance,
		float& OutCrossTrackDistance) const = 0;
};
