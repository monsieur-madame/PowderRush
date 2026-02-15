#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PowderTypes.h"
#include "TerrainManager.generated.h"

class ULevelStreamingDynamic;

USTRUCT(BlueprintType)
struct FZoneDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	EZoneType ZoneType = EZoneType::PowderBowl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	TSoftObjectPtr<UWorld> LevelAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float ZoneLength = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float DifficultyRating = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float BaseWeight = 1.0f;
};

USTRUCT()
struct FActiveZone
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ZoneIndex = -1;

	EZoneType ZoneType = EZoneType::PowderBowl;
	FVector Offset = FVector::ZeroVector;
	float StartDistance = 0.0f;
	float EndDistance = 0.0f;

	UPROPERTY()
	TObjectPtr<ULevelStreamingDynamic> StreamingLevel;
};

UCLASS()
class POWDERRUSH_API ATerrainManager : public AActor
{
	GENERATED_BODY()

public:
	ATerrainManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Terrain")
	void ResetTerrain();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Terrain")
	void SetPlayerDistance(float Distance);

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	EZoneType GetCurrentZoneType() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Terrain")
	ESurfaceType GetSurfaceTypeAtDistance(float Distance) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	TArray<FZoneDefinition> ZoneDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	int32 ZonesAhead = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	int32 ZonesBehind = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Terrain")
	float DifficultyEscalationRate = 0.1f;

	TArray<FActiveZone> ActiveZones;
	float PlayerDistance = 0.0f;
	float TotalGeneratedDistance = 0.0f;
	int32 ZonesGenerated = 0;

	EZoneType SelectNextZoneType();
	void SpawnZoneAhead();
	void UnloadZoneBehind();
	float GetZoneWeight(const FZoneDefinition& Zone) const;

	ESurfaceType GetDefaultSurfaceForZone(EZoneType Type) const;
};
