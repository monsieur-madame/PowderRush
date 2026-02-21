#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderCoursePath.generated.h"

class USceneComponent;
class USplineComponent;
class UStaticMesh;
class UMaterialInterface;
class UHierarchicalInstancedStaticMeshComponent;
class APowderFinishLine;

UCLASS()
class POWDERRUSH_API APowderCoursePath : public AActor
{
	GENERATED_BODY()

public:
	APowderCoursePath();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void SnapPointsToPowderTerrain();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void ResampleSplineUniformly();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void ReverseSplineDirection();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void PlaceFinishLineAtEnd();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void GenerateBoundaryTrees();

	UFUNCTION(CallInEditor, Category = "PowderRush|Terrain|Course|Tools")
	void ClearBoundaryTrees();

	/** Author-facing identifier for this course path. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	FName CourseName = FName(TEXT("DefaultCourse"));

	/** Distance offset from spline start used for spawn/start placement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course", meta = (ClampMin = "0.0"))
	float StartDistanceOffset = 0.0f;

	/** Distance to move uphill when respawning after wipeout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course", meta = (ClampMin = "0.0"))
	float RespawnBacktrackDistance = 1500.0f;

	/**
	 * Normalized weather breakpoints across course progress [0..1].
	 * Expected default: 0.25, 0.55, 0.80.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course")
	TArray<float> WeatherBreakpointsNormalized = { 0.25f, 0.55f, 0.80f };

	/** Vertical trace distance used by SnapPointsToPowderTerrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Tools", meta = (ClampMin = "500.0"))
	float SnapTraceDistance = 50000.0f;

	/** Desired spacing when rebuilding the spline with uniform distance points. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Tools", meta = (ClampMin = "50.0"))
	float ResampleStepDistance = 500.0f;

	/** Finish line actor class to spawn from PlaceFinishLineAtEnd. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Finish")
	TSubclassOf<APowderFinishLine> FinishLineClass;

	/** Width passed to APowderFinishLine::InitExtent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Finish", meta = (ClampMin = "100.0"))
	float FinishLineWidth = 5000.0f;

	/** Distance back from spline end where the finish line is placed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Finish", meta = (ClampMin = "0.0"))
	float FinishLineDistanceFromEnd = 200.0f;

	/** Vertical offset applied when placing finish line from spline sample point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Finish")
	float FinishLineHeightOffset = 20.0f;

	/** If true, delete existing APowderFinishLine actors in level before placing a new one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Finish")
	bool bReplaceExistingFinishLines = true;

	/** Tree meshes used by boundary generation along both spline edges. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary")
	TArray<TObjectPtr<UStaticMesh>> BoundaryTreeMeshes;

	/** Optional material override for boundary tree HISM components. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary")
	TObjectPtr<UMaterialInterface> BoundaryTreeMaterialOverride;

	/** Distance from spline centerline to each boundary side. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "100.0"))
	float BoundaryEdgeOffset = 2600.0f;

	/** Distance between boundary tree samples along spline. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "100.0"))
	float BoundaryTreeSpacing = 700.0f;

	/** Random forward/back jitter from each sample point. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "0.0"))
	float BoundaryAlongJitter = 160.0f;

	/** Random lateral jitter around boundary offset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "0.0"))
	float BoundaryLateralJitter = 120.0f;

	/** Random yaw variation for boundary trees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float BoundaryYawJitter = 35.0f;

	/** Additional placement offset along terrain normal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary")
	float BoundaryHeightOffset = 0.0f;

	/** Minimum terrain normal Z accepted for placement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float BoundaryMinNormalZ = 0.2f;

	/** Upward trace distance for boundary tree placement traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "100.0"))
	float BoundaryTraceUp = 3000.0f;

	/** Downward trace distance for boundary tree placement traces. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary", meta = (ClampMin = "1000.0"))
	float BoundaryTraceDown = 12000.0f;

	/** Seed used for deterministic boundary generation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary")
	int32 BoundarySeed = 1337;

	/** If true, previous generated boundary trees are removed before generating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain|Course|Boundary")
	bool bReplaceExistingBoundaryTrees = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<USplineComponent> CourseSpline;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> BoundaryTreeHISMComponents;
};
