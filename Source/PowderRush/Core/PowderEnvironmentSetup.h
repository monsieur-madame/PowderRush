#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderEnvironmentSetup.generated.h"

class APowderSlopeTile;

UCLASS()
class POWDERRUSH_API APowderEnvironmentSetup : public AActor
{
	GENERATED_BODY()

public:
	APowderEnvironmentSetup();

	virtual void BeginPlay() override;

	/** World position at the top-center of the slope surface */
	FVector GetSlopeStartPosition() const { return SlopeOrigin; }
	FVector GetSlopeDownhill() const { return SlopeDownhill; }

	// --- Obstacle Density (per million square units) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment|Trees")
	float BorderTreesPerMillion = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment|Trees")
	float CourseTreesPerMillion = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment|Rocks")
	float BorderRocksPerMillion = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment|Rocks")
	float CourseRocksPerMillion = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment|Jumps")
	float JumpsPerMillion = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment")
	int32 EnvironmentSeed = 42;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment")
	float FinishLinePosition = 0.95f;

private:
	void SpawnLighting();
	void SpawnSkyAndFog();
	void SpawnBorderTrees(FRandomStream& RNG);
	void SpawnCourseTrees(FRandomStream& RNG);
	void SpawnBorderRocks(FRandomStream& RNG);
	void SpawnCourseRocks(FRandomStream& RNG);
	void SpawnJumps(FRandomStream& RNG);
	void SpawnFinishLine();
	void ApplySlopeMaterial();

	int32 ComputeCountFromDensity(float PerMillion) const;
	FVector SlopePositionToWorld(float DownhillT, float LateralOffset) const;
	bool IsTooCloseToExisting(const FVector& Location, const TArray<FVector>& Existing, float MinSpacing) const;

	APowderSlopeTile* FindSlopeTile() const;

	// Cached slope properties
	FVector SlopeOrigin = FVector::ZeroVector;
	FVector SlopeDownhill = FVector::ForwardVector;
	FVector SlopeLateral = FVector::RightVector;
	float SlopeLength = 10000.0f;
	float SlopeWidth = 5000.0f;
	float SlopeAngle = 15.0f;

	TArray<FVector> PlacedObstacleLocations;
};
