#include "Terrain/SlopeTile.h"
#include "Components/StaticMeshComponent.h"

APowderSlopeTile::APowderSlopeTile()
{
	PrimaryActorTick.bCanEverTick = false;

	SlopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlopeMesh"));
	SetRootComponent(SlopeMesh);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		SlopeMesh->SetStaticMesh(CubeMesh.Object);
	}

	// Scale: Cube is 100x100x100 base. (100,50,1) => 10000x5000x100 units
	SlopeMesh->SetRelativeScale3D(FVector(SlopeLength / 100.0f, SlopeWidth / 100.0f, 1.0f));

	// Pitch the slab downhill
	SlopeMesh->SetRelativeRotation(FRotator(-SlopeAngleDegrees, 0.0f, 0.0f));

	// Collision for physics and line traces
	SlopeMesh->SetCollisionProfileName(TEXT("BlockAll"));
	SlopeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
