#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowderJump.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;
class UPowderMovementComponent;

UCLASS()
class POWDERRUSH_API APowderJump : public AActor
{
	GENERATED_BODY()

public:
	APowderJump();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float RampLength = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float RampWidth = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float RampHeight = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float LaunchSpeedMultiplier = 0.6f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UStaticMeshComponent> RampMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Terrain")
	TObjectPtr<UBoxComponent> LaunchTrigger;

	UFUNCTION()
	void OnLaunchTriggerOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	float LastLaunchTime = -10.0f;
	float LaunchCooldown = 2.0f;
};
