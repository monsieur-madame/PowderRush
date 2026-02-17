#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderTree.generated.h"

class UStaticMeshComponent;
class USceneComponent;
struct FProceduralTreeParams;

UCLASS()
class POWDERRUSH_API APowderTree : public AActor
{
	GENERATED_BODY()

public:
	APowderTree();

	/** Randomize tree dimensions, foliage layers, snow cap, and colors. */
	void Randomize(FRandomStream& RNG, const FProceduralTreeParams& Params);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float TrunkHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float TrunkRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float FoliageHeight = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float FoliageRadius = 120.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> TrunkMesh;

	/** Primary foliage cone (always visible). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> FoliageMesh;

	/** Additional foliage layers for fuller look (hidden by default). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> FoliageLayer2;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> FoliageLayer3;

	/** Snow cap on top (hidden by default). */
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> SnowCapMesh;
};
