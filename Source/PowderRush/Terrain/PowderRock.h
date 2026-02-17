#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderRock.generated.h"

class UStaticMeshComponent;
class USceneComponent;
struct FProceduralRockParams;

UCLASS()
class POWDERRUSH_API APowderRock : public AActor
{
	GENERATED_BODY()

public:
	APowderRock();

	void RandomizeAppearance(FRandomStream& RNG);

	/** Randomize rock: clusters, snow cover, color variation. */
	void Randomize(FRandomStream& RNG, const FProceduralRockParams& Params);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float BaseSize = 80.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> RockMesh;

	/** Cluster cubes (hidden by default, enabled by Randomize). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ClusterMesh2;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ClusterMesh3;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ClusterMesh4;

	/** Snow cover ellipsoid on top (hidden by default). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SnowCoverMesh;
};
