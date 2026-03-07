#include "Effects/PowderWeatherVolume.h"

#include "Components/BoxComponent.h"
#include "Core/PowderEnvironmentSetup.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

namespace
{
float BlendWrappedAngleDegrees(const TArray<float>& AnglesDeg, const TArray<float>& Weights)
{
	double SinSum = 0.0;
	double CosSum = 0.0;
	for (int32 Idx = 0; Idx < AnglesDeg.Num(); ++Idx)
	{
		const double Radians = FMath::DegreesToRadians(AnglesDeg[Idx]);
		const double W = static_cast<double>(Weights[Idx]);
		SinSum += FMath::Sin(Radians) * W;
		CosSum += FMath::Cos(Radians) * W;
	}

	if (FMath::IsNearlyZero(SinSum) && FMath::IsNearlyZero(CosSum))
	{
		return AnglesDeg.Num() > 0 ? AnglesDeg[0] : 0.0f;
	}

	return FMath::RadiansToDegrees(FMath::Atan2(SinSum, CosSum));
}
} // namespace

APowderWeatherVolume::APowderWeatherVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	VolumeBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("VolumeBounds"));
	SetRootComponent(VolumeBounds);
	VolumeBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VolumeBounds->SetGenerateOverlapEvents(false);
	VolumeBounds->SetBoxExtent(FVector(3000.0f, 3000.0f, 1200.0f));
}

void APowderWeatherVolume::BeginPlay()
{
	Super::BeginPlay();

	CreateSnowfallComponent();

	// Timer-based activation check (every 0.5s) for mobile perf
	if (SnowfallComponent)
	{
		GetWorldTimerManager().SetTimer(
			ActivationTimerHandle, this, &APowderWeatherVolume::UpdateSnowfallActivation,
			0.5f, true);
	}
}

void APowderWeatherVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ActivationTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void APowderWeatherVolume::CreateSnowfallComponent()
{
	if (Config.SnowfallRate <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	UNiagaraSystem* SnowSystem = LoadObject<UNiagaraSystem>(
		nullptr, TEXT("/Game/Effects/NS_Snowfall.NS_Snowfall"));
	if (!SnowSystem)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PowderWeatherVolume: Missing /Game/Effects/NS_Snowfall. "
			     "Snowfall particles disabled for volume '%s'."), *GetName());
		return;
	}

	SnowfallComponent = NewObject<UNiagaraComponent>(this, TEXT("SnowfallComponent"));
	SnowfallComponent->SetAsset(SnowSystem);
	SnowfallComponent->SetAutoActivate(false);
	SnowfallComponent->RegisterComponent();
	SnowfallComponent->AttachToComponent(VolumeBounds, FAttachmentTransformRules::KeepRelativeTransform);

	// Position at the top of the volume box
	const FVector BoxExtent = VolumeBounds->GetScaledBoxExtent();
	SnowfallComponent->SetRelativeLocation(FVector(0.0f, 0.0f, BoxExtent.Z));

	// Set Niagara parameters
	SnowfallComponent->SetVariableFloat(FName("SpawnRate"), Config.SnowfallRate * 200.0f);

	// Pre-compute wind velocity vector
	const float WindRad = FMath::DegreesToRadians(Config.WindDirection);
	const float WindScale = Config.WindStrength * 50.0f;
	const FVector WindVelocity(FMath::Cos(WindRad) * WindScale, FMath::Sin(WindRad) * WindScale, 0.0f);
	SnowfallComponent->SetVariableVec3(FName("WindVelocity"), WindVelocity);

	// SpawnExtent = volume box extent so particles fill the zone area
	SnowfallComponent->SetVariableVec3(FName("SpawnExtent"), BoxExtent);
}

void APowderWeatherVolume::UpdateSnowfallActivation()
{
	if (!SnowfallComponent || !bEnabled)
	{
		if (SnowfallComponent && SnowfallComponent->IsActive())
		{
			SnowfallComponent->Deactivate();
		}
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	const FVector Local = GetActorTransform().InverseTransformPosition(PlayerLoc);
	const FVector Extent = VolumeBounds->GetScaledBoxExtent();
	const FVector Outside(
		FMath::Max(FMath::Abs(Local.X) - Extent.X, 0.0f),
		FMath::Max(FMath::Abs(Local.Y) - Extent.Y, 0.0f),
		FMath::Max(FMath::Abs(Local.Z) - Extent.Z, 0.0f));

	const float ActivationRange = BlendDistance;
	const bool bInRange = Outside.Size() <= ActivationRange;

	if (bInRange && !SnowfallComponent->IsActive())
	{
		SnowfallComponent->Activate();
	}
	else if (!bInRange && SnowfallComponent->IsActive())
	{
		SnowfallComponent->Deactivate();
	}
}

void APowderWeatherVolume::PreviewWeather()
{
#if WITH_EDITOR
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> FoundSetups;
	UGameplayStatics::GetAllActorsOfClass(World, APowderEnvironmentSetup::StaticClass(), FoundSetups);
	if (FoundSetups.Num() > 0)
	{
		APowderEnvironmentSetup* EnvSetup = Cast<APowderEnvironmentSetup>(FoundSetups[0]);
		if (EnvSetup)
		{
			EnvSetup->SetupInEditor(Config.Preset);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PowderWeatherVolume: No APowderEnvironmentSetup found in level for preview."));
	}
#endif
}

float APowderWeatherVolume::ComputeInfluenceWeight(const FVector& WorldLocation) const
{
	if (!bEnabled || !VolumeBounds)
	{
		return 0.0f;
	}

	const FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = VolumeBounds->GetScaledBoxExtent();
	const FVector AbsLocal(FMath::Abs(Local.X), FMath::Abs(Local.Y), FMath::Abs(Local.Z));
	const FVector Outside(
		FMath::Max(AbsLocal.X - Extent.X, 0.0f),
		FMath::Max(AbsLocal.Y - Extent.Y, 0.0f),
		FMath::Max(AbsLocal.Z - Extent.Z, 0.0f));

	const float OutsideDistance = Outside.Size();
	if (OutsideDistance <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	if (BlendDistance <= KINDA_SMALL_NUMBER || OutsideDistance >= BlendDistance)
	{
		return 0.0f;
	}

	return 1.0f - (OutsideDistance / BlendDistance);
}

bool APowderWeatherVolume::GetWeatherAtLocation(UWorld* World, const FVector& WorldLocation, FWeatherConfig& OutConfig)
{
	if (!World)
	{
		return false;
	}

	int32 HighestPriority = TNumericLimits<int32>::Lowest();
	TArray<const APowderWeatherVolume*> ActiveVolumes;
	TArray<float> ActiveWeights;

	for (TActorIterator<APowderWeatherVolume> It(World); It; ++It)
	{
		const APowderWeatherVolume* Volume = *It;
		if (!Volume || !Volume->bEnabled)
		{
			continue;
		}

		const float Weight = Volume->ComputeInfluenceWeight(WorldLocation);
		if (Weight <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		if (Volume->Priority > HighestPriority)
		{
			HighestPriority = Volume->Priority;
			ActiveVolumes.Reset();
			ActiveWeights.Reset();
		}

		if (Volume->Priority == HighestPriority)
		{
			ActiveVolumes.Add(Volume);
			ActiveWeights.Add(Weight);
		}
	}

	if (ActiveVolumes.Num() == 0)
	{
		return false;
	}

	if (ActiveVolumes.Num() == 1)
	{
		OutConfig = ActiveVolumes[0]->Config;
		return true;
	}

	float WeightSum = 0.0f;
	for (const float W : ActiveWeights)
	{
		WeightSum += W;
	}
	if (WeightSum <= KINDA_SMALL_NUMBER)
	{
		OutConfig = ActiveVolumes[0]->Config;
		return true;
	}

	for (float& W : ActiveWeights)
	{
		W /= WeightSum;
	}

	int32 DominantIndex = 0;
	float DominantWeight = ActiveWeights[0];
	for (int32 Idx = 1; Idx < ActiveWeights.Num(); ++Idx)
	{
		if (ActiveWeights[Idx] > DominantWeight)
		{
			DominantWeight = ActiveWeights[Idx];
			DominantIndex = Idx;
		}
	}

	OutConfig = FWeatherConfig();
	OutConfig.Preset = ActiveVolumes[DominantIndex]->Config.Preset;
	OutConfig.SunColor = FLinearColor::Black;
	OutConfig.SunIntensity = 0.0f;
	OutConfig.SkyColor = FLinearColor::Black;
	OutConfig.SkyLightIntensity = 0.0f;
	OutConfig.FogDensity = 0.0f;
	OutConfig.FogColor = FLinearColor::Black;
	OutConfig.FogStartDistance = 0.0f;
	OutConfig.FogHeightFalloff = 0.0f;
	OutConfig.SnowfallRate = 0.0f;
	OutConfig.WindStrength = 0.0f;
	OutConfig.WindDirection = 0.0f;
	OutConfig.TransitionTime = 0.0f;

	TArray<float> SunPitch;
	TArray<float> SunYaw;
	TArray<float> SunRoll;
	SunPitch.Reserve(ActiveVolumes.Num());
	SunYaw.Reserve(ActiveVolumes.Num());
	SunRoll.Reserve(ActiveVolumes.Num());

	for (int32 Idx = 0; Idx < ActiveVolumes.Num(); ++Idx)
	{
		const FWeatherConfig& Config = ActiveVolumes[Idx]->Config;
		const float W = ActiveWeights[Idx];

		OutConfig.SunColor += Config.SunColor * W;
		OutConfig.SunIntensity += Config.SunIntensity * W;
		OutConfig.SkyColor += Config.SkyColor * W;
		OutConfig.SkyLightIntensity += Config.SkyLightIntensity * W;
		OutConfig.FogDensity += Config.FogDensity * W;
		OutConfig.FogColor += Config.FogColor * W;
		OutConfig.FogStartDistance += Config.FogStartDistance * W;
		OutConfig.FogHeightFalloff += Config.FogHeightFalloff * W;
		OutConfig.SnowfallRate += Config.SnowfallRate * W;
		OutConfig.WindStrength += Config.WindStrength * W;
		OutConfig.WindDirection += Config.WindDirection * W;
		OutConfig.TransitionTime += Config.TransitionTime * W;

		SunPitch.Add(Config.SunRotation.Pitch);
		SunYaw.Add(Config.SunRotation.Yaw);
		SunRoll.Add(Config.SunRotation.Roll);
	}

	OutConfig.SunRotation.Pitch = BlendWrappedAngleDegrees(SunPitch, ActiveWeights);
	OutConfig.SunRotation.Yaw = BlendWrappedAngleDegrees(SunYaw, ActiveWeights);
	OutConfig.SunRotation.Roll = BlendWrappedAngleDegrees(SunRoll, ActiveWeights);

	return true;
}

