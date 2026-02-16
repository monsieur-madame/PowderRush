#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderTree.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class POWDERRUSH_API APowderTree : public AActor
{
	GENERATED_BODY()

public:
	APowderTree();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> FoliageMesh;
};
