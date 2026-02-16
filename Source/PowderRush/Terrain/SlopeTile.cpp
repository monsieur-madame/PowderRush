#include "Terrain/SlopeTile.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

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

	// Collision for physics and line traces
	SlopeMesh->SetCollisionProfileName(TEXT("BlockAll"));
	SlopeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APowderSlopeTile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Re-apply scale and rotation from current property values
	// Cube is 100x100x100 base units
	if (SlopeMesh)
	{
		SlopeMesh->SetRelativeScale3D(FVector(SlopeLength / 100.0f, SlopeWidth / 100.0f, 1.0f));
		SlopeMesh->SetRelativeRotation(FRotator(-SlopeAngleDegrees, 0.0f, 0.0f));
	}
}
