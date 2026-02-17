#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PowderTypes.h"
#include "Terrain/PowderSurfaceQueryProvider.h"
#include "TerrainManager.generated.h"

class USceneComponent;
class APowderCoursePath;

struct FRuntimeSplineSample
{
	float Distance = 0.0f;
	FVector Position = FVector::ZeroVector;
	FVector Tangent = FVector::ForwardVector;
	FVector UpVector = FVector::UpVector;
};

UCLASS()
class POWDERRUSH_API ATerrainManager : public AActor, public IPowderSurfaceQueryProvider
{
	GENERATED_BODY()

public:
	ATerrainManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Terrain")
	bool InitializeCourse();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Terrain")
	void ResetTerrain();

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	bool IsCourseInitialized() const { return RuntimeSplineSamples.Num() >= 2; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	float GetPlayerDistance() const { return PlayerDistance; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	float GetCourseLength() const { return CachedCourseLength; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetSlopeStartPosition() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetSlopeDownhill() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetStartDownhill() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FRotator GetStartFacingRotation() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FRotator GetRespawnFacingRotation() const;

	/** Get respawn position slightly uphill from the current progress. */
	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetRespawnPosition() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetPositionAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	FVector GetDirectionAtDistance(float Distance) const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	float ProjectPositionOntoCourse(const FVector& WorldPos) const;

	virtual bool SampleSurfaceAtWorldPosition(
		const FVector& WorldPos,
		FSurfaceProperties& OutSurface,
		float& OutCourseDistance) const override;

	virtual bool SampleCourseFrameAtWorldPosition(
		const FVector& WorldPos,
		FVector& OutTangent,
		FVector& OutUp,
		float& OutCourseDistance,
		float& OutCrossTrackDistance) const override;

	/** Get the surface properties at the player's current course distance. */
	const FSurfaceProperties* GetCurrentSurfaceProperties() const;

	/** Get the surface properties nearest to an arbitrary course distance. */
	const FSurfaceProperties* GetSurfacePropertiesAtDistance(float Distance) const;

	const TArray<float>& GetWeatherBreakpointsNormalized() const { return CachedWeatherBreakpointsNormalized; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	bool bAutoInitializeCourseOnBeginPlay = true;

	/** Distance step used when sampling the placed course-path spline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course", meta = (ClampMin = "100.0"))
	float CoursePathSampleStep = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	float StartDistanceOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	float SpawnHeightOffset = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	float RespawnBacktrackDistance = 1500.0f;

	/** Additional facing offset applied when converting spline direction to pawn yaw. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	float SpawnFacingYawOffset = 0.0f;

private:
	void ClearRuntimeComponents();
	APowderCoursePath* FindFirstUsableCoursePath() const;
	bool BuildRuntimeSamplesFromCoursePathSpline(APowderCoursePath* PreferredPath = nullptr);
	void UpdatePlayerDistance();
	int32 FindSegmentIndexForDistance(float Distance) const;
	FVector GetUpVectorAtDistance(float Distance) const;
	void UpdateProjectionHint(int32 SegmentIndex) const;
	FRotator BuildFacingRotationFromDirection(const FVector& Direction) const;

	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SceneRoot;
	TArray<FRuntimeSplineSample> RuntimeSplineSamples;
	TArray<float> CachedWeatherBreakpointsNormalized;
	float CachedCourseLength = 0.0f;
	float PlayerDistance = 0.0f;
	mutable int32 LastProjectedSegmentIndex = INDEX_NONE;
	FSurfaceProperties DefaultSurfaceProperties;
};
