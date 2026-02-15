#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObstacleBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EObstacleType : uint8
{
	Tree,
	Rock,
	Gate,
	Ramp
};

UCLASS(Abstract, Blueprintable)
class POWDERRUSH_API AObstacleBase : public AActor
{
	GENERATED_BODY()

public:
	AObstacleBase();

	UFUNCTION(BlueprintPure, Category = "PowderRush|Obstacle")
	EObstacleType GetObstacleType() const { return ObstacleType; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Obstacle")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	// Inner collision - hitting this causes wipeout
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Obstacle")
	TObjectPtr<USphereComponent> CollisionComp;

	// Outer shell - overlapping this triggers near-miss scoring
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Obstacle")
	TObjectPtr<USphereComponent> NearMissComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacle")
	EObstacleType ObstacleType = EObstacleType::Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacle")
	float NearMissRadius = 200.0f;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnNearMissBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
