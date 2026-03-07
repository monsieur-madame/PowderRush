// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PowderGameMode.h"
#include "PowderAvalancheComponent.generated.h"

class ATerrainManager;
class APowderAvalancheActor;

UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderAvalancheComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPowderAvalancheComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Reset avalanche position and elapsed time for a new run. */
	void ResetAvalanche();

	UFUNCTION(BlueprintPure, Category = "PowderRush|Avalanche")
	float GetAvalancheDistance() const { return AvalancheDistance; }

	/** Absolute gap between player and avalanche (cm). */
	UFUNCTION(BlueprintPure, Category = "PowderRush|Avalanche")
	float GetGapDistance() const;

	/** Normalized gap: 0 = caught, 1 = safe (clamped). */
	UFUNCTION(BlueprintPure, Category = "PowderRush|Avalanche")
	float GetGapNormalized() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Avalanche")
	bool IsAvalancheActive() const { return bIsActive; }

	// --- Tuning ---

	/** How far behind the player the avalanche starts (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float StartOffset = 5000.0f;

	/** Initial avalanche speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float BaseSpeed = 500.0f;

	/** Speed increase per second of run time (cm/s^2). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float AccelerationRate = 10.0f;

	/** Maximum avalanche speed (cm/s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float MaxSpeed = 2500.0f;

	/** How close the avalanche must be to trigger wipeout (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float CatchDistance = 100.0f;

	/** Gap distance considered "safe" for normalization (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Avalanche")
	float SafeDistance = 5000.0f;

protected:
	UFUNCTION()
	void OnRunStateChanged(EPowderRunState NewState);

	void SpawnVisual();
	void DestroyVisual();
	void UpdateVisualPosition();

private:
	float AvalancheDistance = 0.0f;
	float ElapsedRunTime = 0.0f;
	bool bIsActive = false;

	UPROPERTY()
	TObjectPtr<ATerrainManager> CachedTerrainManager;

	UPROPERTY()
	TObjectPtr<APowderAvalancheActor> AvalancheVisual;
};
