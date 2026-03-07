// Copyright PowderRush. All Rights Reserved.

#include "Terrain/PowderAvalancheActor.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "UObject/ConstructorHelpers.h"

APowderAvalancheActor::APowderAvalancheActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = WallMesh;

	// Use engine's built-in plane mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneFinder.Succeeded())
	{
		WallMesh->SetStaticMesh(PlaneFinder.Object);
	}

	// Rotate plane to stand vertical (plane default is horizontal, rotate 90 around X)
	// and scale to desired wall dimensions (plane is 100x100 units by default)
	WallMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	WallMesh->SetRelativeScale3D(FVector(WallWidth / 100.0f, WallHeight / 100.0f, 1.0f));

	// No collision — purely visual
	WallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WallMesh->SetCastShadow(false);
}

void APowderAvalancheActor::UpdatePosition(const FVector& WorldPos, const FVector& CourseDir)
{
	// Create translucent material on first call (can't create MIDs in constructor)
	if (!WallMID)
	{
		WallMID = PowderMaterialHelper::CreateTranslucentColorMID(
			this,
			FLinearColor(0.85f, 0.88f, 0.95f),  // Slightly blue-white snow cloud
			0.6f);
		if (WallMID)
		{
			WallMesh->SetMaterial(0, WallMID);
		}
	}

	// Position the wall at the avalanche location, raised by half the wall height
	SetActorLocation(WorldPos + FVector(0.0f, 0.0f, WallHeight * 0.5f));

	// Face along the course direction (wall perpendicular to course)
	if (!CourseDir.IsNearlyZero())
	{
		SetActorRotation(CourseDir.Rotation());
	}
}
