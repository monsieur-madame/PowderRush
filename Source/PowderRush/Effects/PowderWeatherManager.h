#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PowderTypes.h"
#include "PowderWeatherManager.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UNiagaraComponent;

/**
 * Manages weather transitions: smoothly lerps lighting, fog, sky,
 * and controls ambient snowfall particle rate.
 * Attach to APowderEnvironmentSetup after it spawns the lighting/fog/sky.
 */
UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderWeatherManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPowderWeatherManager();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize with references to spawned environment components. */
	void Initialize(
		UDirectionalLightComponent* DirLight,
		UExponentialHeightFogComponent* Fog,
		USkyLightComponent* SkyLight,
		UStaticMeshComponent* SkyDome,
		UMaterialInstanceDynamic* SkyDomeMID);

	/** Begin transitioning to a new weather config. */
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Weather")
	void SetWeather(const FWeatherConfig& NewConfig);

	/** Set weather immediately (no transition). */
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Weather")
	void SetWeatherImmediate(const FWeatherConfig& NewConfig);

	/** Get a default config for a preset. */
	UFUNCTION(BlueprintPure, Category = "PowderRush|Weather")
	static FWeatherConfig GetDefaultConfig(EWeatherPreset Preset);

	/** Current weather state (exposed for dev menu). */
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Weather")
	FWeatherConfig CurrentWeather;

private:
	void ApplyWeatherState(const FWeatherConfig& Config);
	void LerpWeatherState(float Alpha);

	// Transition state
	bool bIsTransitioning = false;
	float TransitionAlpha = 0.0f;
	float TransitionDuration = 5.0f;
	FWeatherConfig TransitionStart;
	FWeatherConfig TransitionTarget;

	// Cached environment refs (set via Initialize)
	UPROPERTY()
	TObjectPtr<UDirectionalLightComponent> CachedDirLight;

	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> CachedFog;

	UPROPERTY()
	TObjectPtr<USkyLightComponent> CachedSkyLight;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CachedSkyDome;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedSkyDomeMID;

	// Ambient snowfall Niagara component (spawned if SnowfallRate > 0)
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SnowfallComponent;
};
