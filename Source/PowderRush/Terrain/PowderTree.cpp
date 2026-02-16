#include "Terrain/PowderTree.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

APowderTree::APowderTree()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root scene component — trunk and foliage attach independently
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Trunk: cylinder
	TrunkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Trunk"));
	TrunkMesh->SetupAttachment(Root);
	TrunkMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TrunkMesh->SetCollisionObjectType(ECC_WorldStatic);
	TrunkMesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderAsset.Succeeded())
	{
		TrunkMesh->SetStaticMesh(CylinderAsset.Object);
	}

	// Cylinder is 100 units tall, radius 50 by default
	float TrunkScaleXY = TrunkRadius / 50.0f;
	float TrunkScaleZ = TrunkHeight / 100.0f;
	TrunkMesh->SetRelativeScale3D(FVector(TrunkScaleXY, TrunkScaleXY, TrunkScaleZ));

	// Foliage: cone on top of trunk
	FoliageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Foliage"));
	FoliageMesh->SetupAttachment(Root);
	FoliageMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeAsset(
		TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeAsset.Succeeded())
	{
		FoliageMesh->SetStaticMesh(ConeAsset.Object);
	}

	// Cone is 100 units tall, radius 50 by default
	float FoliageScaleXY = FoliageRadius / 50.0f;
	float FoliageScaleZ = FoliageHeight / 100.0f;
	FoliageMesh->SetRelativeScale3D(FVector(FoliageScaleXY, FoliageScaleXY, FoliageScaleZ));

	// Position foliage on top of trunk (both in root space, no parent scale issues)
	// Trunk center is at origin, top is at TrunkHeight/2
	// Foliage center should be at TrunkHeight/2 + FoliageHeight/2
	float FoliageCenterZ = (TrunkHeight * 0.5f) + (FoliageHeight * 0.5f);
	FoliageMesh->SetRelativeLocation(FVector(0.0f, 0.0f, FoliageCenterZ));

	// Apply colored materials
	TrunkMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.35f, 0.2f, 0.1f)));
	FoliageMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.1f, 0.35f, 0.1f)));
}
