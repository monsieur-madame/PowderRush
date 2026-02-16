#include "Core/PowderEnvironmentSetup.h"
#include "Terrain/SlopeTile.h"
#include "Terrain/PowderTree.h"
#include "Terrain/PowderRock.h"
#include "Terrain/PowderJump.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Effects/PowderMaterialHelper.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

APowderEnvironmentSetup::APowderEnvironmentSetup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APowderEnvironmentSetup::BeginPlay()
{
	Super::BeginPlay();

	// Find the slope tile to derive coordinate system
	APowderSlopeTile* Tile = FindSlopeTile();
	if (Tile)
	{
		SlopeLength = Tile->GetSlopeLength();
		SlopeWidth = Tile->GetSlopeWidth();
		SlopeAngle = Tile->GetSlopeAngle();

		// Downhill direction: forward rotated down by slope angle
		float AngleRad = FMath::DegreesToRadians(SlopeAngle);
		SlopeDownhill = FVector(FMath::Cos(AngleRad), 0.0f, -FMath::Sin(AngleRad));
		SlopeLateral = FVector::RightVector;

		// Surface normal points "up" out of the tilted slab
		FVector SurfaceNormal = FVector(FMath::Sin(AngleRad), 0.0f, FMath::Cos(AngleRad));

		// Offset origin to uphill edge, on top of the slab surface
		// The cube is 100 units thick (Z scale 1.0), so the surface is 50 units above centerline
		SlopeOrigin = Tile->GetActorLocation()
			- SlopeDownhill * (SlopeLength * 0.5f)
			+ SurfaceNormal * 50.0f;
	}

	FRandomStream RNG(EnvironmentSeed);

	SpawnLighting();
	SpawnSkyAndFog();
	SpawnBorderTrees(RNG);
	SpawnCourseTrees(RNG);
	SpawnBorderRocks(RNG);
	SpawnCourseRocks(RNG);
	SpawnJumps(RNG);
	ApplySlopeMaterial();
}

APowderSlopeTile* APowderEnvironmentSetup::FindSlopeTile() const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APowderSlopeTile::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		return Cast<APowderSlopeTile>(Found[0]);
	}
	return nullptr;
}

FVector APowderEnvironmentSetup::SlopePositionToWorld(float DownhillT, float LateralOffset) const
{
	// DownhillT: 0.0 = top of slope, 1.0 = bottom
	// LateralOffset: -1.0 = full left, 1.0 = full right (in units of half-width)
	return SlopeOrigin
		+ SlopeDownhill * (DownhillT * SlopeLength)
		+ SlopeLateral * (LateralOffset * SlopeWidth * 0.5f);
}

int32 APowderEnvironmentSetup::ComputeCountFromDensity(float PerMillion) const
{
	float Area = SlopeLength * SlopeWidth;
	return FMath::Clamp(FMath::RoundToInt(Area * PerMillion / 1000000.0f), 1, 500);
}

bool APowderEnvironmentSetup::IsTooCloseToExisting(const FVector& Location, const TArray<FVector>& Existing, float MinSpacing) const
{
	for (const FVector& Pos : Existing)
	{
		if (FVector::Dist(Location, Pos) < MinSpacing)
		{
			return true;
		}
	}
	return false;
}

void APowderEnvironmentSetup::SpawnLighting()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADirectionalLight* DirLight = World->SpawnActor<ADirectionalLight>(
		FVector::ZeroVector, FRotator(-40.0f, -60.0f, 0.0f), Params);
	if (DirLight)
	{
		UDirectionalLightComponent* DirLightComp = Cast<UDirectionalLightComponent>(DirLight->GetLightComponent());
		if (DirLightComp)
		{
			DirLightComp->SetAtmosphereSunLight(true);
			DirLightComp->SetIntensity(4.0f);
			DirLightComp->SetLightColor(FColor(255, 248, 230));
			DirLightComp->SetCastShadows(true);
		}
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

	// Sky dome — large inverted sphere as reliable sky backdrop
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

		// Two-sided unlit sky blue material — visible from inside the sphere
		DomeMesh->SetMaterial(0, PowderMaterialHelper::CreateSkyDomeMID(DomeActor, FLinearColor(0.4f, 0.65f, 0.95f)));
	}

	// Sky light for ambient fill
	ASkyLight* SkyLightActor = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (SkyLightActor)
	{
		USkyLightComponent* SkyLightComp = SkyLightActor->GetLightComponent();
		if (SkyLightComp)
		{
			SkyLightComp->SetIntensity(1.0f);
			SkyLightComp->bRealTimeCapture = true;
			SkyLightComp->RecaptureSky();
		}
	}

	// Exponential height fog
	AExponentialHeightFog* FogActor = World->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (FogActor)
	{
		UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
		if (FogComp)
		{
			FogComp->SetFogDensity(0.002f);
			FogComp->SetFogInscatteringColor(FLinearColor(0.7f, 0.8f, 0.95f));
			FogComp->FogHeightFalloff = 0.2f;
			FogComp->SetStartDistance(2000.0f);
		}
	}
}

void APowderEnvironmentSetup::SpawnBorderTrees(FRandomStream& RNG)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Count = ComputeCountFromDensity(BorderTreesPerMillion);
	const float MinSpacing = 300.0f;
	const float BandStart = 0.7f;
	const float BandEnd = 1.3f;

	for (int32 i = 0; i < Count; ++i)
	{
		float DownhillT = RNG.FRandRange(0.0f, 1.0f);
		float Side = (RNG.FRand() < 0.5f) ? -1.0f : 1.0f;
		float LateralT = Side * RNG.FRandRange(BandStart, BandEnd);

		FVector Location = SlopePositionToWorld(DownhillT, LateralT);

		if (IsTooCloseToExisting(Location, PlacedObstacleLocations, MinSpacing))
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderTree* Tree = World->SpawnActor<APowderTree>(Location, FRotator::ZeroRotator, Params);
		if (Tree)
		{
			float Yaw = RNG.FRandRange(0.0f, 360.0f);
			Tree->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
			float ScaleVar = RNG.FRandRange(0.7f, 1.3f);
			Tree->SetActorScale3D(FVector(ScaleVar));

			PlacedObstacleLocations.Add(Location);
		}
	}
}

void APowderEnvironmentSetup::SpawnCourseTrees(FRandomStream& RNG)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Count = ComputeCountFromDensity(CourseTreesPerMillion);
	const float MinSpacing = 500.0f;
	const float BandStart = 0.0f;
	const float BandEnd = 0.5f;

	for (int32 i = 0; i < Count; ++i)
	{
		// Avoid top 10% of slope so player start area is clear
		float DownhillT = RNG.FRandRange(0.1f, 1.0f);
		float Side = (RNG.FRand() < 0.5f) ? -1.0f : 1.0f;
		float LateralT = Side * RNG.FRandRange(BandStart, BandEnd);

		FVector Location = SlopePositionToWorld(DownhillT, LateralT);

		if (IsTooCloseToExisting(Location, PlacedObstacleLocations, MinSpacing))
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderTree* Tree = World->SpawnActor<APowderTree>(Location, FRotator::ZeroRotator, Params);
		if (Tree)
		{
			float Yaw = RNG.FRandRange(0.0f, 360.0f);
			Tree->SetActorRotation(FRotator(0.0f, Yaw, 0.0f));
			float ScaleVar = RNG.FRandRange(0.6f, 1.1f);
			Tree->SetActorScale3D(FVector(ScaleVar));

			PlacedObstacleLocations.Add(Location);
		}
	}
}

void APowderEnvironmentSetup::SpawnBorderRocks(FRandomStream& RNG)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Count = ComputeCountFromDensity(BorderRocksPerMillion);
	const float MinSpacing = 200.0f;
	const float BandStart = 0.6f;
	const float BandEnd = 1.1f;

	for (int32 i = 0; i < Count; ++i)
	{
		float DownhillT = RNG.FRandRange(0.05f, 0.95f);
		float Side = (RNG.FRand() < 0.5f) ? -1.0f : 1.0f;
		float LateralT = Side * RNG.FRandRange(BandStart, BandEnd);

		FVector Location = SlopePositionToWorld(DownhillT, LateralT);

		if (IsTooCloseToExisting(Location, PlacedObstacleLocations, MinSpacing))
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderRock* Rock = World->SpawnActor<APowderRock>(Location, FRotator::ZeroRotator, Params);
		if (Rock)
		{
			Rock->RandomizeAppearance(RNG);
			PlacedObstacleLocations.Add(Location);
		}
	}
}

void APowderEnvironmentSetup::SpawnCourseRocks(FRandomStream& RNG)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Count = ComputeCountFromDensity(CourseRocksPerMillion);
	const float MinSpacing = 300.0f;
	const float BandStart = 0.0f;
	const float BandEnd = 0.4f;

	for (int32 i = 0; i < Count; ++i)
	{
		float DownhillT = RNG.FRandRange(0.1f, 0.95f);
		float Side = (RNG.FRand() < 0.5f) ? -1.0f : 1.0f;
		float LateralT = Side * RNG.FRandRange(BandStart, BandEnd);

		FVector Location = SlopePositionToWorld(DownhillT, LateralT);

		if (IsTooCloseToExisting(Location, PlacedObstacleLocations, MinSpacing))
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderRock* Rock = World->SpawnActor<APowderRock>(Location, FRotator::ZeroRotator, Params);
		if (Rock)
		{
			Rock->RandomizeAppearance(RNG);
			PlacedObstacleLocations.Add(Location);
		}
	}
}

void APowderEnvironmentSetup::SpawnJumps(FRandomStream& RNG)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Count = ComputeCountFromDensity(JumpsPerMillion);
	const float MinSpacing = 800.0f;
	const float BandStart = 0.0f;
	const float BandEnd = 0.3f;

	// Orient ramp facing downhill
	FRotator RampRotation = SlopeDownhill.Rotation();

	for (int32 i = 0; i < Count; ++i)
	{
		// Avoid top 15% of slope
		float DownhillT = RNG.FRandRange(0.15f, 0.9f);
		float Side = (RNG.FRand() < 0.5f) ? -1.0f : 1.0f;
		float LateralT = Side * RNG.FRandRange(BandStart, BandEnd);

		FVector Location = SlopePositionToWorld(DownhillT, LateralT);

		// Check spacing against all obstacles (jumps, trees, rocks)
		if (IsTooCloseToExisting(Location, PlacedObstacleLocations, MinSpacing))
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderJump* Jump = World->SpawnActor<APowderJump>(Location, RampRotation, Params);
		if (Jump)
		{
			PlacedObstacleLocations.Add(Location);
		}
	}
}

void APowderEnvironmentSetup::ApplySlopeMaterial()
{
	APowderSlopeTile* Tile = FindSlopeTile();
	if (!Tile)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Tile->GetSlopeMesh();
	if (!Mesh)
	{
		return;
	}

	Mesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.92f, 0.94f, 0.98f)));
}
