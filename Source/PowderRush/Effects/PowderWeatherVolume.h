#pragma once

#include "CoreMinimal.h"
#include "Core/PowderTypes.h"
#include "GameFramework/Actor.h"
#include "PowderWeatherVolume.generated.h"

class UBoxComponent;

UCLASS()
class POWDERRUSH_API APowderWeatherVolume : public AActor
{
	GENERATED_BODY()

public:
	APowderWeatherVolume();

	/** Computes weather at world position by blending active volumes of highest priority. */
	static bool GetWeatherAtLocation(UWorld* World, const FVector& WorldLocation, FWeatherConfig& OutConfig);

	/** Whether this volume participates in weather evaluation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather|Volume")
	bool bEnabled = true;

	/** Higher priority volumes override lower priority volumes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather|Volume")
	int32 Priority = 0;

	/** Blend distance outside the volume bounds (in world units). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather|Volume", meta = (ClampMin = "0.0"))
	float BlendDistance = 2000.0f;

	/** Weather config contributed by this volume. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather|Volume")
	FWeatherConfig Config;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Weather|Volume")
	TObjectPtr<UBoxComponent> VolumeBounds;

private:
	float ComputeInfluenceWeight(const FVector& WorldLocation) const;
};

