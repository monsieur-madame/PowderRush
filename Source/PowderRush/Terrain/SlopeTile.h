#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlopeTile.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable)
class POWDERRUSH_API APowderSlopeTile : public AActor
{
	GENERATED_BODY()

public:
	APowderSlopeTile();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	UStaticMeshComponent* GetSlopeMesh() const { return SlopeMesh; }

	float GetSlopeAngle() const { return SlopeAngleDegrees; }
	float GetSlopeLength() const { return SlopeLength; }
	float GetSlopeWidth() const { return SlopeWidth; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> SlopeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float SlopeLength = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float SlopeWidth = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float SlopeAngleDegrees = 15.0f;
};
