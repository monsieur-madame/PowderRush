#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderRock.generated.h"

class UStaticMeshComponent;

UCLASS()
class POWDERRUSH_API APowderRock : public AActor
{
	GENERATED_BODY()

public:
	APowderRock();

	void RandomizeAppearance(FRandomStream& RNG);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float BaseSize = 80.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> RockMesh;
};
