#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PowderTypes.h"
#include "PowderEnvironmentSetup.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UPowderWeatherManager;

/**
 * Spawns global environment elements: lighting, sky dome, fog.
 * Terrain/obstacle content is authored separately as static level content.
 */
UCLASS()
class POWDERRUSH_API APowderEnvironmentSetup : public AActor
{
	GENERATED_BODY()

public:
	APowderEnvironmentSetup();

	virtual void BeginPlay() override;

	/** Editor-time setup: spawns lighting/sky/fog and applies a weather preset without creating a WeatherManager. */
	void SetupInEditor(EWeatherPreset Preset = EWeatherPreset::ClearDay);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment")
	bool bReuseExistingSceneLights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Environment")
	float DynamicShadowDistance = 8000.0f;

	// Accessors for weather manager
	UDirectionalLightComponent* GetDirectionalLight() const { return CachedDirLight; }
	UExponentialHeightFogComponent* GetFogComponent() const { return CachedFog; }
	USkyLightComponent* GetSkyLight() const { return CachedSkyLight; }
	UStaticMeshComponent* GetSkyDomeMesh() const { return CachedSkyDome; }
	UMaterialInstanceDynamic* GetSkyDomeMID() const { return SkyDomeMID; }
	UPowderWeatherManager* GetWeatherManager() const { return WeatherManager; }

private:
	void SpawnLighting();
	void SpawnSkyAndFog();

	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> CachedDirLight;

	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> CachedFog;

	UPROPERTY()
	TObjectPtr<USkyLightComponent> CachedSkyLight;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedSkyDome;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> SkyDomeMID;

	UPROPERTY()
	TObjectPtr<UPowderWeatherManager> WeatherManager;
};
