#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoinPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

UCLASS(Blueprintable)
class POWDERRUSH_API ACoinPickup : public AActor
{
	GENERATED_BODY()

public:
	ACoinPickup();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Pickup")
	TObjectPtr<USphereComponent> CollectionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Pickup")
	TObjectPtr<UStaticMeshComponent> CoinMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Pickup")
	TObjectPtr<UNiagaraComponent> SparkleEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Pickup")
	float RotationSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Pickup")
	float BobAmplitude = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Pickup")
	float BobSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Pickup")
	int32 CoinValue = 1;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Collect(AActor* Collector);

	float InitialZ = 0.0f;
	float TimeAccumulator = 0.0f;
	bool bCollected = false;
};
