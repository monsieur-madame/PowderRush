#include "Effects/PowderWeatherManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UPowderWeatherManager::UPowderWeatherManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	CurrentWeather = GetDefaultConfig(EWeatherPreset::ClearDay);
}

void UPowderWeatherManager::Initialize(
	UDirectionalLightComponent* DirLight,
	UExponentialHeightFogComponent* Fog,
	USkyLightComponent* SkyLight,
	UStaticMeshComponent* SkyDome,
	UMaterialInstanceDynamic* SkyDomeMID)
{
	CachedDirLight = DirLight;
	CachedFog = Fog;
	CachedSkyLight = SkyLight;
	CachedSkyDome = SkyDome;
	CachedSkyDomeMID = SkyDomeMID;

	// Apply initial weather immediately
	ApplyWeatherState(CurrentWeather);
}

void UPowderWeatherManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTransitioning)
	{
		return;
	}

	TransitionAlpha = FMath::Clamp(TransitionAlpha + DeltaTime / TransitionDuration, 0.0f, 1.0f);
	LerpWeatherState(TransitionAlpha);

	if (TransitionAlpha >= 1.0f)
	{
		bIsTransitioning = false;
		CurrentWeather = TransitionTarget;
	}
}

void UPowderWeatherManager::SetWeather(const FWeatherConfig& NewConfig)
{
	TransitionStart = CurrentWeather;
	TransitionTarget = NewConfig;
	TransitionDuration = FMath::Max(NewConfig.TransitionTime, KINDA_SMALL_NUMBER);
	TransitionAlpha = 0.0f;
	bIsTransitioning = true;
}

void UPowderWeatherManager::SetWeatherImmediate(const FWeatherConfig& NewConfig)
{
	CurrentWeather = NewConfig;
	bIsTransitioning = false;
	ApplyWeatherState(CurrentWeather);
}

void UPowderWeatherManager::LerpWeatherState(float Alpha)
{
	FWeatherConfig Lerped;
	Lerped.SunColor = FMath::Lerp(TransitionStart.SunColor, TransitionTarget.SunColor, Alpha);
	Lerped.SunIntensity = FMath::Lerp(TransitionStart.SunIntensity, TransitionTarget.SunIntensity, Alpha);
	Lerped.SunRotation = FMath::Lerp(TransitionStart.SunRotation, TransitionTarget.SunRotation, Alpha);
	Lerped.SkyColor = FMath::Lerp(TransitionStart.SkyColor, TransitionTarget.SkyColor, Alpha);
	Lerped.SkyLightIntensity = FMath::Lerp(TransitionStart.SkyLightIntensity, TransitionTarget.SkyLightIntensity, Alpha);
	Lerped.FogDensity = FMath::Lerp(TransitionStart.FogDensity, TransitionTarget.FogDensity, Alpha);
	Lerped.FogColor = FMath::Lerp(TransitionStart.FogColor, TransitionTarget.FogColor, Alpha);
	Lerped.FogStartDistance = FMath::Lerp(TransitionStart.FogStartDistance, TransitionTarget.FogStartDistance, Alpha);
	Lerped.FogHeightFalloff = FMath::Lerp(TransitionStart.FogHeightFalloff, TransitionTarget.FogHeightFalloff, Alpha);
	Lerped.SnowfallRate = FMath::Lerp(TransitionStart.SnowfallRate, TransitionTarget.SnowfallRate, Alpha);
	Lerped.WindStrength = FMath::Lerp(TransitionStart.WindStrength, TransitionTarget.WindStrength, Alpha);
	Lerped.WindDirection = FMath::Lerp(TransitionStart.WindDirection, TransitionTarget.WindDirection, Alpha);

	CurrentWeather = Lerped;
	ApplyWeatherState(Lerped);
}

void UPowderWeatherManager::ApplyWeatherState(const FWeatherConfig& Config)
{
	// Directional light (sun)
	if (CachedDirLight)
	{
		CachedDirLight->SetIntensity(Config.SunIntensity);
		CachedDirLight->SetLightColor(Config.SunColor.ToFColor(true));
		CachedDirLight->SetWorldRotation(Config.SunRotation);
	}

	// Sky dome material
	if (CachedSkyDomeMID)
	{
		CachedSkyDomeMID->SetVectorParameterValue(FName("Color"), Config.SkyColor);
	}

	// Sky light
	if (CachedSkyLight)
	{
		CachedSkyLight->SetIntensity(Config.SkyLightIntensity);
	}

	// Fog
	if (CachedFog)
	{
		CachedFog->SetFogDensity(Config.FogDensity);
		CachedFog->SetFogInscatteringColor(Config.FogColor);
		CachedFog->FogHeightFalloff = Config.FogHeightFalloff;
		CachedFog->SetStartDistance(Config.FogStartDistance);
	}
}

FWeatherConfig UPowderWeatherManager::GetDefaultConfig(EWeatherPreset Preset)
{
	FWeatherConfig Config;
	Config.Preset = Preset;

	switch (Preset)
	{
	case EWeatherPreset::ClearDay:
		Config.SunColor = FLinearColor(1.0f, 0.97f, 0.90f);
		Config.SunIntensity = 4.0f;
		Config.SunRotation = FRotator(-40.0f, -60.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.4f, 0.65f, 0.95f);
		Config.SkyLightIntensity = 1.0f;
		Config.FogDensity = 0.002f;
		Config.FogColor = FLinearColor(0.7f, 0.8f, 0.95f);
		Config.FogStartDistance = 2000.0f;
		Config.FogHeightFalloff = 0.2f;
		Config.SnowfallRate = 0.0f;
		Config.WindStrength = 0.0f;
		break;

	case EWeatherPreset::Overcast:
		Config.SunColor = FLinearColor(0.8f, 0.82f, 0.85f);
		Config.SunIntensity = 2.0f;
		Config.SunRotation = FRotator(-35.0f, -60.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.5f, 0.55f, 0.6f);
		Config.SkyLightIntensity = 1.5f;
		Config.FogDensity = 0.005f;
		Config.FogColor = FLinearColor(0.6f, 0.65f, 0.7f);
		Config.FogStartDistance = 1500.0f;
		Config.FogHeightFalloff = 0.15f;
		Config.SnowfallRate = 0.5f;
		Config.WindStrength = 1.0f;
		break;

	case EWeatherPreset::Snowfall:
		Config.SunColor = FLinearColor(0.7f, 0.72f, 0.78f);
		Config.SunIntensity = 1.5f;
		Config.SunRotation = FRotator(-30.0f, -60.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.45f, 0.5f, 0.55f);
		Config.SkyLightIntensity = 1.8f;
		Config.FogDensity = 0.008f;
		Config.FogColor = FLinearColor(0.65f, 0.68f, 0.75f);
		Config.FogStartDistance = 800.0f;
		Config.FogHeightFalloff = 0.1f;
		Config.SnowfallRate = 3.0f;
		Config.WindStrength = 3.0f;
		break;

	case EWeatherPreset::Blizzard:
		Config.SunColor = FLinearColor(0.5f, 0.52f, 0.58f);
		Config.SunIntensity = 0.8f;
		Config.SunRotation = FRotator(-25.0f, -60.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.35f, 0.38f, 0.42f);
		Config.SkyLightIntensity = 2.0f;
		Config.FogDensity = 0.02f;
		Config.FogColor = FLinearColor(0.6f, 0.62f, 0.68f);
		Config.FogStartDistance = 300.0f;
		Config.FogHeightFalloff = 0.05f;
		Config.SnowfallRate = 8.0f;
		Config.WindStrength = 8.0f;
		break;

	case EWeatherPreset::Sunset:
		Config.SunColor = FLinearColor(1.0f, 0.6f, 0.3f);
		Config.SunIntensity = 3.0f;
		Config.SunRotation = FRotator(-10.0f, -80.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.6f, 0.4f, 0.5f);
		Config.SkyLightIntensity = 0.8f;
		Config.FogDensity = 0.004f;
		Config.FogColor = FLinearColor(0.8f, 0.5f, 0.35f);
		Config.FogStartDistance = 1500.0f;
		Config.FogHeightFalloff = 0.15f;
		Config.SnowfallRate = 0.0f;
		Config.WindStrength = 0.5f;
		break;

	case EWeatherPreset::Twilight:
		Config.SunColor = FLinearColor(0.3f, 0.3f, 0.5f);
		Config.SunIntensity = 0.5f;
		Config.SunRotation = FRotator(-5.0f, -90.0f, 0.0f);
		Config.SkyColor = FLinearColor(0.15f, 0.15f, 0.3f);
		Config.SkyLightIntensity = 0.4f;
		Config.FogDensity = 0.006f;
		Config.FogColor = FLinearColor(0.2f, 0.2f, 0.35f);
		Config.FogStartDistance = 1000.0f;
		Config.FogHeightFalloff = 0.12f;
		Config.SnowfallRate = 0.0f;
		Config.WindStrength = 0.0f;
		break;
	}

	return Config;
}

