#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PowderSnowSpray.generated.h"

class UStaticMeshComponent;
class UStaticMesh;

UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderSnowSpray : public USceneComponent
{
	GENERATED_BODY()

public:
	UPowderSnowSpray();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ActivateSpray(float CarveDirection);
	void DeactivateSpray();
	bool IsSprayActive() const { return bIsActive; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	int32 PoolSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float ParticleLifetime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float SpawnRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float LaunchSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float UpwardBias = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float SpreadAngle = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float ParticleGravity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	float ParticleStartScale = 0.08f;

private:
	struct FSnowParticle
	{
		TObjectPtr<UStaticMeshComponent> Mesh;
		FVector Velocity;
		float TimeRemaining;
		float StartScale;
		bool bActive;
	};

	TArray<FSnowParticle> ParticlePool;
	bool bIsActive = false;
	float CarveDir = 0.0f;
	float SpawnAccumulator = 0.0f;

	TObjectPtr<UStaticMesh> SphereMesh;

	void InitPool();
	void SpawnParticle();
	void UpdateParticles(float DeltaTime);
	int32 FindFreeSlot() const;
};
