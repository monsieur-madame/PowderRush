#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderFinishLine.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class POWDERRUSH_API APowderFinishLine : public AActor
{
	GENERATED_BODY()

public:
	APowderFinishLine();

	void InitExtent(float SlopeWidth);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
