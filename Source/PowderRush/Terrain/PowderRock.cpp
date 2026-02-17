#include "Terrain/PowderRock.h"
#include "Core/PowderTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

APowderRock::APowderRock()
{
	PrimaryActorTick.bCanEverTick = false;
	Tags.AddUnique(FName(TEXT("PowderObstacle")));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// Primary rock cube
	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	RockMesh->SetupAttachment(Root);
	RockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RockMesh->SetCollisionObjectType(ECC_WorldStatic);
	RockMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RockMesh->ComponentTags.AddUnique(FName(TEXT("PowderObstacle")));
	if (CubeAsset.Succeeded())
	{
		RockMesh->SetStaticMesh(CubeAsset.Object);
	}
	float S = BaseSize / 100.0f;
	RockMesh->SetRelativeScale3D(FVector(S * 1.2f, S * 0.8f, S * 0.6f));
	RockMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.4f, 0.4f, 0.42f)));

	// Cluster cubes (hidden by default)
	auto CreateCluster = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Cluster = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Cluster->SetupAttachment(Root);
		Cluster->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Cluster->SetCollisionObjectType(ECC_WorldStatic);
		Cluster->SetCollisionResponseToAllChannels(ECR_Block);
		if (CubeAsset.Succeeded())
		{
			Cluster->SetStaticMesh(CubeAsset.Object);
		}
		Cluster->SetVisibility(false);
		return Cluster;
	};

	ClusterMesh2 = CreateCluster(TEXT("Cluster2"));
	ClusterMesh3 = CreateCluster(TEXT("Cluster3"));
	ClusterMesh4 = CreateCluster(TEXT("Cluster4"));

	// Snow cover sphere (hidden by default)
	SnowCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnowCover"));
	SnowCoverMesh->SetupAttachment(Root);
	SnowCoverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (SphereAsset.Succeeded())
	{
		SnowCoverMesh->SetStaticMesh(SphereAsset.Object);
	}
	SnowCoverMesh->SetVisibility(false);
}

void APowderRock::RandomizeAppearance(FRandomStream& RNG)
{
	float S = BaseSize / 100.0f;
	float SX = S * RNG.FRandRange(0.6f, 1.4f);
	float SY = S * RNG.FRandRange(0.5f, 1.2f);
	float SZ = S * RNG.FRandRange(0.4f, 0.9f);
	RockMesh->SetRelativeScale3D(FVector(SX, SY, SZ));

	FRotator RandomRot(
		RNG.FRandRange(-15.0f, 15.0f),
		RNG.FRandRange(0.0f, 360.0f),
		RNG.FRandRange(-10.0f, 10.0f));
	RockMesh->SetRelativeRotation(RandomRot);
}

void APowderRock::Randomize(FRandomStream& RNG, const FProceduralRockParams& Params)
{
	BaseSize = RNG.FRandRange(Params.BaseSizeRange.X, Params.BaseSizeRange.Y);

	// Color with variation
	FLinearColor Color = Params.BaseColor;
	Color.R += RNG.FRandRange(-Params.ColorVariation.R, Params.ColorVariation.R);
	Color.G += RNG.FRandRange(-Params.ColorVariation.G, Params.ColorVariation.G);
	Color.B += RNG.FRandRange(-Params.ColorVariation.B, Params.ColorVariation.B);
	UMaterialInstanceDynamic* RockMID = PowderMaterialHelper::CreateColorMID(this, Color);

	// Randomize primary rock
	float S = BaseSize / 100.0f;
	float SX = S * RNG.FRandRange(Params.ScaleXRange.X, Params.ScaleXRange.Y);
	float SY = S * RNG.FRandRange(Params.ScaleYRange.X, Params.ScaleYRange.Y);
	float SZ = S * RNG.FRandRange(Params.ScaleZRange.X, Params.ScaleZRange.Y);
	RockMesh->SetRelativeScale3D(FVector(SX, SY, SZ));
	RockMesh->SetRelativeRotation(FRotator(
		RNG.FRandRange(-15.0f, 15.0f),
		RNG.FRandRange(0.0f, 360.0f),
		RNG.FRandRange(-10.0f, 10.0f)));
	RockMesh->SetMaterial(0, RockMID);

	// Cluster generation
	UStaticMeshComponent* Clusters[3] = { ClusterMesh2, ClusterMesh3, ClusterMesh4 };
	bool bMakeCluster = RNG.FRand() < Params.ClusterChance;
	int32 ClusterCount = bMakeCluster ? RNG.RandRange(1, FMath::Min(Params.MaxClusterCount - 1, 3)) : 0;

	for (int32 i = 0; i < 3; ++i)
	{
		UStaticMeshComponent* Cluster = Clusters[i];
		if (!Cluster)
		{
			continue;
		}

		if (i >= ClusterCount)
		{
			Cluster->SetVisibility(false);
			Cluster->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			continue;
		}

		Cluster->SetVisibility(true);
		Cluster->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// Smaller cluster piece nearby
		float ClusterSize = BaseSize * RNG.FRandRange(0.4f, 0.8f) / 100.0f;
		float CSX = ClusterSize * RNG.FRandRange(Params.ScaleXRange.X, Params.ScaleXRange.Y);
		float CSY = ClusterSize * RNG.FRandRange(Params.ScaleYRange.X, Params.ScaleYRange.Y);
		float CSZ = ClusterSize * RNG.FRandRange(Params.ScaleZRange.X, Params.ScaleZRange.Y);
		Cluster->SetRelativeScale3D(FVector(CSX, CSY, CSZ));

		// Offset from center
		float OffsetDist = BaseSize * RNG.FRandRange(0.6f, 1.2f);
		float OffsetAngle = RNG.FRandRange(0.0f, 360.0f);
		float OffsetX = OffsetDist * FMath::Cos(FMath::DegreesToRadians(OffsetAngle));
		float OffsetY = OffsetDist * FMath::Sin(FMath::DegreesToRadians(OffsetAngle));
		Cluster->SetRelativeLocation(FVector(OffsetX, OffsetY, RNG.FRandRange(-20.0f, 10.0f)));
		Cluster->SetRelativeRotation(FRotator(
			RNG.FRandRange(-20.0f, 20.0f),
			RNG.FRandRange(0.0f, 360.0f),
			RNG.FRandRange(-15.0f, 15.0f)));

		// Slightly different color per cluster piece
		FLinearColor ClusterColor = Color;
		ClusterColor.R += RNG.FRandRange(-0.05f, 0.05f);
		ClusterColor.G += RNG.FRandRange(-0.04f, 0.04f);
		ClusterColor.B += RNG.FRandRange(-0.03f, 0.03f);
		Cluster->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, ClusterColor));
	}

	// Snow cover on top (white ellipsoid)
	if (SnowCoverMesh)
	{
		bool bShowSnow = RNG.FRand() < Params.SnowCoverChance;
		SnowCoverMesh->SetVisibility(bShowSnow);

		if (bShowSnow)
		{
			// Flatten sphere on top of the rock
			float SnowScaleXY = SX * 1.1f;
			float SnowScaleZ = SZ * 0.3f;
			SnowCoverMesh->SetRelativeScale3D(FVector(SnowScaleXY, SnowScaleXY, SnowScaleZ));
			float SnowZ = BaseSize * SZ * 0.5f + SnowScaleZ * 50.0f * 0.3f;
			SnowCoverMesh->SetRelativeLocation(FVector(0.0f, 0.0f, SnowZ));
			SnowCoverMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.92f, 0.94f, 0.98f)));
		}
	}
}
