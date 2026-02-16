#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PowderTypes.h"
#include "PowderPowerup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class POWDERRUSH_API APowderPowerup : public AActor
{
	GENERATED_BODY()

public:
	APowderPowerup();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Powerup")
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Powerup")
	TObjectPtr<UStaticMeshComponent> PowerupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup")
	EPowerupType PowerupType = EPowerupType::SpeedBoost;

	// --- Tuning ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup|SpeedBoost")
	float BoostSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup|SpeedBoost")
	float BoostDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup|ScoreMultiplier")
	float ScoreMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup|ScoreMultiplier")
	float MultiplierDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup")
	int32 CollectPoints = 50;

	// --- Bob/Rotate ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup")
	float RotationSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup")
	float BobAmplitude = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Powerup")
	float BobSpeed = 2.5f;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Collect(AActor* Collector);

	float InitialZ = 0.0f;
	float TimeAccumulator = 0.0f;
	bool bCollected = false;
};
