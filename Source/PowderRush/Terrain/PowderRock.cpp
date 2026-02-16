#include "Terrain/PowderRock.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

APowderRock::APowderRock()
{
	PrimaryActorTick.bCanEverTick = false;

	RockMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RockMesh"));
	SetRootComponent(RockMesh);
	RockMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RockMesh->SetCollisionObjectType(ECC_WorldStatic);
	RockMesh->SetCollisionResponseToAllChannels(ECR_Block);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		RockMesh->SetStaticMesh(CubeAsset.Object);
	}

	// Default non-uniform scale for angular look
	float S = BaseSize / 100.0f;
	RockMesh->SetRelativeScale3D(FVector(S * 1.2f, S * 0.8f, S * 0.6f));

	// Apply gray material
	RockMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.4f, 0.4f, 0.42f)));
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
