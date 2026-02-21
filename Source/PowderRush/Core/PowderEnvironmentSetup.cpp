#include "Core/PowderEnvironmentSetup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Effects/PowderMaterialHelper.h"
#include "Effects/PowderWeatherManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"

APowderEnvironmentSetup::APowderEnvironmentSetup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APowderEnvironmentSetup::BeginPlay()
{
	Super::BeginPlay();

	SpawnLighting();
	SpawnSkyAndFog();

	// Create weather manager and initialize with cached refs
	WeatherManager = NewObject<UPowderWeatherManager>(this, TEXT("WeatherManager"));
	if (WeatherManager)
	{
		WeatherManager->RegisterComponent();
		WeatherManager->Initialize(CachedDirLight, CachedFog, CachedSkyLight, CachedSkyDome, SkyDomeMID);
	}
}

void APowderEnvironmentSetup::SetupInEditor(EWeatherPreset Preset)
{
#if WITH_EDITOR
	SpawnLighting();
	SpawnSkyAndFog();

	const FWeatherConfig Config = UPowderWeatherManager::GetDefaultConfig(Preset);

	if (CachedDirLight)
	{
		CachedDirLight->SetIntensity(Config.SunIntensity);
		CachedDirLight->SetLightColor(Config.SunColor.ToFColor(true));
		CachedDirLight->SetWorldRotation(Config.SunRotation);
	}

	if (SkyDomeMID)
	{
		SkyDomeMID->SetVectorParameterValue(TEXT("Color"), Config.SkyColor);
	}

	if (CachedSkyLight)
	{
		CachedSkyLight->SetIntensity(Config.SkyLightIntensity);
		CachedSkyLight->RecaptureSky();
	}

	if (CachedFog)
	{
		CachedFog->SetFogDensity(Config.FogDensity);
		CachedFog->SetFogInscatteringColor(Config.FogColor);
		CachedFog->FogHeightFalloff = Config.FogHeightFalloff;
		CachedFog->SetStartDistance(Config.FogStartDistance);
	}
#endif
}

void APowderEnvironmentSetup::SpawnLighting()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bReuseExistingSceneLights)
	{
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			ADirectionalLight* Existing = *It;
			if (!Existing)
			{
				continue;
			}

			CachedDirLight = Cast<UDirectionalLightComponent>(Existing->GetLightComponent());
			if (CachedDirLight)
			{
				break;
			}
		}
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!CachedDirLight)
	{
		ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(
			FVector::ZeroVector, FRotator(-40.0f, -60.0f, 0.0f), Params);
		if (DirLight)
		{
			DirLight->GetRootComponent()->SetMobility(EComponentMobility::Stationary);
			CachedDirLight = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent());
		}
	}

	if (CachedDirLight)
	{
		CachedDirLight->SetAtmosphereSunLight(true);
		CachedDirLight->SetIntensity(4.0f);
		CachedDirLight->SetLightColor(FColor(255, 248, 230));
		CachedDirLight->SetCastShadows(true);
		CachedDirLight->DynamicShadowDistanceMovableLight = DynamicShadowDistance;
		CachedDirLight->DynamicShadowDistanceStationaryLight = DynamicShadowDistance;
	}
}

void APowderEnvironmentSetup::SpawnSkyAndFog()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Sky dome
	AActor* DomeActor = World->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (DomeActor)
	{
		UStaticMeshComponent* DomeMesh = NewObject<UStaticMeshComponent>(DomeActor, TEXT("SkyDomeMesh"));
		DomeActor->SetRootComponent(DomeMesh);
		DomeMesh->RegisterComponent();

		UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		if (SphereMesh)
		{
			DomeMesh->SetStaticMesh(SphereMesh);
		}

		DomeMesh->SetWorldScale3D(FVector(5000.0f));
		DomeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DomeMesh->CastShadow = false;

		SkyDomeMID = PowderMaterialHelper::CreateSkyDomeMID(DomeActor, FLinearColor(0.4f, 0.65f, 0.95f));
		DomeMesh->SetMaterial(0, SkyDomeMID);
		CachedSkyDome = DomeMesh;
	}

	if (bReuseExistingSceneLights)
	{
		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			if (ASkyLight* Existing = *It)
			{
				CachedSkyLight = Existing->GetLightComponent();
				if (CachedSkyLight)
				{
					break;
				}
			}
		}
	}

	if (!CachedSkyLight)
	{
		ASkyLight* SkyLightActor = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (SkyLightActor)
		{
			SkyLightActor->GetRootComponent()->SetMobility(EComponentMobility::Stationary);
			CachedSkyLight = SkyLightActor->GetLightComponent();
		}
	}

	if (CachedSkyLight)
	{
		CachedSkyLight->SetIntensity(1.0f);
		CachedSkyLight->bRealTimeCapture = false;
		CachedSkyLight->RecaptureSky();
	}

	if (bReuseExistingSceneLights)
	{
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			if (AExponentialHeightFog* Existing = *It)
			{
				CachedFog = Existing->GetComponent();
				if (CachedFog)
				{
					break;
				}
			}
		}
	}

	if (!CachedFog)
	{
		AExponentialHeightFog* FogActor = World->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (FogActor)
		{
			FogActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
			CachedFog = FogActor->GetComponent();
		}
	}

	if (CachedFog)
	{
		CachedFog->SetFogDensity(0.002f);
		CachedFog->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 0.95f));
		CachedFog->FogHeightFalloff = 0.2f;
		CachedFog->SetStartDistance(2000.0f);
	}
}
