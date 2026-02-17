#include "Terrain/PowderTree.h"
#include "Core/PowderTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

APowderTree::APowderTree()
{
	PrimaryActorTick.bCanEverTick = false;
	Tags.AddUnique(FName(TEXT("PowderObstacle")));

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeAsset(
		TEXT("/Engine/BasicShapes/Cone.Cone"));

	// Trunk: cylinder
	TrunkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Trunk"));
	TrunkMesh->SetupAttachment(Root);
	TrunkMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TrunkMesh->SetCollisionObjectType(ECC_WorldStatic);
	TrunkMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TrunkMesh->ComponentTags.AddUnique(FName(TEXT("PowderObstacle")));
	if (CylinderAsset.Succeeded())
	{
		TrunkMesh->SetStaticMesh(CylinderAsset.Object);
	}
	float TrunkScaleXY = TrunkRadius / 50.0f;
	float TrunkScaleZ = TrunkHeight / 100.0f;
	TrunkMesh->SetRelativeScale3D(FVector(TrunkScaleXY, TrunkScaleXY, TrunkScaleZ));

	// Primary foliage cone
	FoliageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Foliage"));
	FoliageMesh->SetupAttachment(Root);
	FoliageMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ConeAsset.Succeeded())
	{
		FoliageMesh->SetStaticMesh(ConeAsset.Object);
	}
	float FoliageScaleXY = FoliageRadius / 50.0f;
	float FoliageScaleZ = FoliageHeight / 100.0f;
	FoliageMesh->SetRelativeScale3D(FVector(FoliageScaleXY, FoliageScaleXY, FoliageScaleZ));
	float FoliageCenterZ = (TrunkHeight * 0.5f) + (FoliageHeight * 0.5f);
	FoliageMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FoliageCenterZ));

	// Additional foliage layers (hidden by default, enabled by Randomize)
	auto CreateFoliageLayer = [&](const TCHAR* Name) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Layer = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Layer->SetupAttachment(Root);
		Layer->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (ConeAsset.Succeeded())
		{
			Layer->SetStaticMesh(ConeAsset.Object);
		}
		Layer->SetVisibility(false);
		return Layer;
	};

	FoliageLayer2 = CreateFoliageLayer(TEXT("FoliageLayer2"));
	FoliageLayer3 = CreateFoliageLayer(TEXT("FoliageLayer3"));

	// Snow cap (hidden by default)
	SnowCapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SnowCap"));
	SnowCapMesh->SetupAttachment(Root);
	SnowCapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (ConeAsset.Succeeded())
	{
		SnowCapMesh->SetStaticMesh(ConeAsset.Object);
	}
	SnowCapMesh->SetVisibility(false);

	// Apply default materials
	TrunkMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.35f, 0.2f, 0.1f)));
	FoliageMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.1f, 0.35f, 0.1f)));
}

void APowderTree::Randomize(FRandomStream& RNG, const FProceduralTreeParams& Params)
{
	// Randomize trunk dimensions
	TrunkHeight = RNG.FRandRange(Params.TrunkHeightRange.X, Params.TrunkHeightRange.Y);
	TrunkRadius = RNG.FRandRange(Params.TrunkRadiusRange.X, Params.TrunkRadiusRange.Y);
	FoliageHeight = RNG.FRandRange(Params.FoliageHeightRange.X, Params.FoliageHeightRange.Y);
	FoliageRadius = RNG.FRandRange(Params.FoliageRadiusRange.X, Params.FoliageRadiusRange.Y);

	// Apply trunk
	if (TrunkMesh)
	{
		float TrunkScaleXY = TrunkRadius / 50.0f;
		float TrunkScaleZ = TrunkHeight / 100.0f;
		TrunkMesh->SetRelativeScale3D(FVector(TrunkScaleXY, TrunkScaleXY, TrunkScaleZ));
		TrunkMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, Params.TrunkColor));
	}

	// Determine foliage layer count (1-MaxFoliageLayers)
	int32 LayerCount = FMath::Clamp(RNG.RandRange(1, Params.MaxFoliageLayers), 1, 3);

	// Foliage color with slight random variation per tree
	FLinearColor FoliageCol = Params.FoliageColor;
	FoliageCol.R += RNG.FRandRange(-0.03f, 0.03f);
	FoliageCol.G += RNG.FRandRange(-0.05f, 0.05f);
	FoliageCol.B += RNG.FRandRange(-0.02f, 0.02f);

	UMaterialInstanceDynamic* FoliageMID = PowderMaterialHelper::CreateColorMID(this, FoliageCol);

	// Stack foliage cones from bottom to top, each smaller than the one below
	UStaticMeshComponent* Layers[3] = { FoliageMesh, FoliageLayer2, FoliageLayer3 };
	float CurrentZ = TrunkHeight * 0.5f;  // Start at top of trunk

	for (int32 i = 0; i < 3; ++i)
	{
		UStaticMeshComponent* Layer = Layers[i];
		if (!Layer)
		{
			continue;
		}

		if (i >= LayerCount)
		{
			Layer->SetVisibility(false);
			continue;
		}

		Layer->SetVisibility(true);

		// Each successive layer is smaller and stacked higher
		float LayerFrac = 1.0f - (static_cast<float>(i) * 0.25f);
		float LayerRadius = FoliageRadius * LayerFrac;
		float LayerHeight = FoliageHeight * LayerFrac;

		float ScaleXY = LayerRadius / 50.0f;
		float ScaleZ = LayerHeight / 100.0f;
		Layer->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));

		float LayerCenterZ = CurrentZ + LayerHeight * 0.5f;
		Layer->SetRelativeLocation(FVector(0.0f, 0.0f, LayerCenterZ));
		Layer->SetMaterial(0, FoliageMID);

		// Overlap layers by 30% for a fuller look
		CurrentZ += LayerHeight * 0.7f;
	}

	// Snow cap on top (white cone)
	if (SnowCapMesh)
	{
		bool bShowSnowCap = RNG.FRand() < Params.SnowCapChance;
		SnowCapMesh->SetVisibility(bShowSnowCap);

		if (bShowSnowCap)
		{
			float CapRadius = FoliageRadius * 0.4f;
			float CapHeight = FoliageHeight * 0.25f;
			float CapScaleXY = CapRadius / 50.0f;
			float CapScaleZ = CapHeight / 100.0f;
			SnowCapMesh->SetRelativeScale3D(FVector(CapScaleXY, CapScaleXY, CapScaleZ));
			SnowCapMesh->SetRelativeLocation(FVector(0.0f, 0.0f, CurrentZ + CapHeight * 0.3f));
			SnowCapMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.95f, 0.95f, 1.0f)));
		}
	}
}
