// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderAvalancheActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Visual representation of the avalanche: a large translucent wall of snow/cloud
 * that advances along the course behind the player.
 */
UCLASS(NotPlaceable)
class POWDERRUSH_API APowderAvalancheActor : public AActor
{
	GENERATED_BODY()

public:
	APowderAvalancheActor();

	/** Update position and orientation to face along the course direction. */
	void UpdatePosition(const FVector& WorldPos, const FVector& CourseDir);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float WallWidth = 4000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float WallHeight = 1500.0f;

protected:
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WallMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> WallMID;
};
