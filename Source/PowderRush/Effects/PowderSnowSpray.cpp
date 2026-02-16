#include "Effects/PowderSnowSpray.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UPowderSnowSpray::UPowderSnowSpray()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereAsset.Succeeded())
	{
		SphereMesh = SphereAsset.Object;
	}
}

void UPowderSnowSpray::BeginPlay()
{
	Super::BeginPlay();
	InitPool();
}

void UPowderSnowSpray::InitPool()
{
	AActor* Owner = GetOwner();
	if (!Owner || !SphereMesh)
	{
		return;
	}

	ParticlePool.SetNum(PoolSize);
	for (int32 i = 0; i < PoolSize; ++i)
	{
		FSnowParticle& P = ParticlePool[i];
		P.Mesh = NewObject<UStaticMeshComponent>(Owner);
		P.Mesh->SetStaticMesh(SphereMesh);
		P.Mesh->SetWorldScale3D(FVector(ParticleStartScale));
		P.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		P.Mesh->SetCastShadow(false);
		P.Mesh->SetVisibility(false);
		P.Mesh->RegisterComponent();

		// Try to apply white material
		UMaterialInstanceDynamic* MID = P.Mesh->CreateAndSetMaterialInstanceDynamic(0);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.95f, 1.0f));
		}

		P.bActive = false;
		P.TimeRemaining = 0.0f;
		P.Velocity = FVector::ZeroVector;
		P.StartScale = ParticleStartScale;
	}
}

void UPowderSnowSpray::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsActive)
	{
		SpawnAccumulator += DeltaTime * SpawnRate;
		while (SpawnAccumulator >= 1.0f)
		{
			SpawnParticle();
			SpawnAccumulator -= 1.0f;
		}
	}

	UpdateParticles(DeltaTime);
}

void UPowderSnowSpray::ActivateSpray(float CarveDirection)
{
	bIsActive = true;
	CarveDir = CarveDirection;
	SpawnAccumulator = 0.0f;
	SetComponentTickEnabled(true);
}

void UPowderSnowSpray::DeactivateSpray()
{
	bIsActive = false;
	// Keep ticking until all particles expire
}

void UPowderSnowSpray::SpawnParticle()
{
	int32 Slot = FindFreeSlot();
	if (Slot == INDEX_NONE)
	{
		return;
	}

	FSnowParticle& P = ParticlePool[Slot];

	// Spawn at component's world location
	FVector SpawnLoc = GetComponentLocation();
	P.Mesh->SetWorldLocation(SpawnLoc);
	P.Mesh->SetWorldScale3D(FVector(ParticleStartScale));
	P.Mesh->SetVisibility(true);

	// Direction: lateral opposite to carve, with upward bias and random spread
	float LateralDir = (CarveDir > 0.0f) ? -1.0f : 1.0f;
	float RandomSpread = FMath::FRandRange(-SpreadAngle, SpreadAngle);
	float SpreadRad = FMath::DegreesToRadians(RandomSpread);

	FVector LaunchDir;
	LaunchDir.X = FMath::Sin(SpreadRad) * LaunchSpeed * 0.3f;
	LaunchDir.Y = LateralDir * LaunchSpeed;
	LaunchDir.Z = UpwardBias + FMath::FRandRange(0.0f, UpwardBias * 0.5f);

	P.Velocity = LaunchDir;
	P.TimeRemaining = ParticleLifetime + FMath::FRandRange(-0.1f, 0.1f);
	P.StartScale = ParticleStartScale;
	P.bActive = true;
}

void UPowderSnowSpray::UpdateParticles(float DeltaTime)
{
	bool bAnyActive = false;

	for (FSnowParticle& P : ParticlePool)
	{
		if (!P.bActive)
		{
			continue;
		}

		P.TimeRemaining -= DeltaTime;
		if (P.TimeRemaining <= 0.0f)
		{
			P.bActive = false;
			P.Mesh->SetVisibility(false);
			continue;
		}

		bAnyActive = true;

		// Apply gravity
		P.Velocity.Z -= ParticleGravity * DeltaTime;

		// Move
		FVector CurrentLoc = P.Mesh->GetComponentLocation();
		P.Mesh->SetWorldLocation(CurrentLoc + P.Velocity * DeltaTime);

		// Shrink over lifetime
		float LifeFraction = P.TimeRemaining / ParticleLifetime;
		float Scale = P.StartScale * LifeFraction;
		P.Mesh->SetWorldScale3D(FVector(FMath::Max(Scale, 0.01f)));
	}

	// Stop ticking when inactive and no particles alive
	if (!bIsActive && !bAnyActive)
	{
		SetComponentTickEnabled(false);
	}
}

int32 UPowderSnowSpray::FindFreeSlot() const
{
	for (int32 i = 0; i < ParticlePool.Num(); ++i)
	{
		if (!ParticlePool[i].bActive)
		{
			return i;
		}
	}
	return INDEX_NONE;
}
