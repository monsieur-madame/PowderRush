#include "Terrain/PowderCoursePath.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Effects/PowderWeatherManager.h"
#include "Effects/PowderWeatherVolume.h"
#include "Terrain/PowderFinishLine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/UObjectGlobals.h"

namespace
{
const FName PowderObstacleTag(TEXT("PowderObstacle"));
const FName PowderGeneratedTag(TEXT("PowderGenerated"));
const FName PowderBoundaryTag(TEXT("PowderBoundaryTree"));
const FName PowderWeatherZoneTag(TEXT("PowderWeatherZone"));

bool FindPowderTerrainHit(
	UWorld* World,
	const AActor* IgnoredActor,
	const FVector& Start,
	const FVector& End,
	FHitResult& OutTerrainHit)
{
	if (!World)
	{
		return false;
	}

	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams;
	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	if (!World->LineTraceMultiByChannel(Hits, Start, End, ECC_WorldStatic, QueryParams))
	{
		return false;
	}

	Hits.Sort([](const FHitResult& A, const FHitResult& B)
	{
		return A.Distance < B.Distance;
	});

	for (const FHitResult& Hit : Hits)
	{
		const UPrimitiveComponent* HitComp = Hit.GetComponent();
		const AActor* HitActor = Hit.GetActor();
		const bool bIsTerrain =
			(HitComp && HitComp->ComponentHasTag(FName(TEXT("PowderTerrain"))))
			|| (HitActor && HitActor->ActorHasTag(FName(TEXT("PowderTerrain"))));
		if (!bIsTerrain)
		{
			continue;
		}

		OutTerrainHit = Hit;
		return true;
	}

	return false;
}
} // namespace

APowderCoursePath::APowderCoursePath()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	CourseSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CourseSpline"));
	CourseSpline->SetupAttachment(Root);
	CourseSpline->SetClosedLoop(false);
	CourseSpline->SetDrawDebug(false);

	FinishLineClass = APowderFinishLine::StaticClass();

	WeatherZonePresets = { EWeatherPreset::ClearDay, EWeatherPreset::Overcast, EWeatherPreset::Snowfall };
}

void APowderCoursePath::SnapPointsToPowderTerrain()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const int32 NumPoints = CourseSpline->GetNumberOfSplinePoints();
	if (NumPoints < 2)
	{
		return;
	}

	Modify();
	CourseSpline->Modify();

	const float TraceDistance = FMath::Max(500.0f, SnapTraceDistance);
	constexpr float TraceStartOffset = 100.0f;
	for (int32 Idx = 0; Idx < NumPoints; ++Idx)
	{
		const FVector CurrentWorld = CourseSpline->GetLocationAtSplinePoint(Idx, ESplineCoordinateSpace::World);
		const FVector Start = CurrentWorld + FVector::UpVector * TraceStartOffset;
		const FVector End = CurrentWorld - FVector::UpVector * TraceDistance;

		FHitResult TerrainHit;
		if (FindPowderTerrainHit(World, this, Start, End, TerrainHit))
		{
			CourseSpline->SetLocationAtSplinePoint(Idx, TerrainHit.ImpactPoint, ESplineCoordinateSpace::World, false);
		}
	}

	CourseSpline->UpdateSpline();
#endif
}

void APowderCoursePath::ResampleSplineUniformly()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	const float Length = CourseSpline->GetSplineLength();
	if (Length <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float Step = FMath::Max(50.0f, ResampleStepDistance);
	const int32 SegmentCount = FMath::Max(1, FMath::CeilToInt(Length / Step));
	TArray<FVector> SampledLocalPoints;
	SampledLocalPoints.Reserve(SegmentCount + 1);

	const FTransform ActorTransform = GetActorTransform();
	for (int32 Idx = 0; Idx <= SegmentCount; ++Idx)
	{
		const float Distance = FMath::Min(Length, static_cast<float>(Idx) * Step);
		const FVector WorldPos = CourseSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		SampledLocalPoints.Add(ActorTransform.InverseTransformPosition(WorldPos));
	}

	Modify();
	CourseSpline->Modify();
	CourseSpline->ClearSplinePoints(false);

	for (int32 Idx = 0; Idx < SampledLocalPoints.Num(); ++Idx)
	{
		CourseSpline->AddSplinePoint(SampledLocalPoints[Idx], ESplineCoordinateSpace::Local, false);
		CourseSpline->SetSplinePointType(Idx, ESplinePointType::Curve, false);
	}

	CourseSpline->SetClosedLoop(false, false);
	CourseSpline->UpdateSpline();
#endif
}

void APowderCoursePath::ReverseSplineDirection()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	const int32 NumPoints = CourseSpline->GetNumberOfSplinePoints();
	if (NumPoints < 2)
	{
		return;
	}

	TArray<FVector> LocalPoints;
	TArray<ESplinePointType::Type> PointTypes;
	LocalPoints.Reserve(NumPoints);
	PointTypes.Reserve(NumPoints);
	for (int32 Idx = 0; Idx < NumPoints; ++Idx)
	{
		LocalPoints.Add(CourseSpline->GetLocationAtSplinePoint(Idx, ESplineCoordinateSpace::Local));
		PointTypes.Add(CourseSpline->GetSplinePointType(Idx));
	}

	Modify();
	CourseSpline->Modify();
	CourseSpline->ClearSplinePoints(false);

	for (int32 Idx = NumPoints - 1; Idx >= 0; --Idx)
	{
		CourseSpline->AddSplinePoint(LocalPoints[Idx], ESplineCoordinateSpace::Local, false);
		const int32 NewIdx = CourseSpline->GetNumberOfSplinePoints() - 1;
		CourseSpline->SetSplinePointType(NewIdx, PointTypes[Idx], false);
	}

	CourseSpline->SetClosedLoop(false, false);
	CourseSpline->UpdateSpline();
#endif
}

void APowderCoursePath::PlaceFinishLineAtEnd()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CourseSpline->GetNumberOfSplinePoints() < 2)
	{
		return;
	}

	UClass* SpawnClass = FinishLineClass ? FinishLineClass.Get() : APowderFinishLine::StaticClass();
	if (!SpawnClass)
	{
		return;
	}

	if (bReplaceExistingFinishLines)
	{
		for (TActorIterator<APowderFinishLine> It(World); It; ++It)
		{
			APowderFinishLine* Existing = *It;
			if (Existing)
			{
				Existing->Destroy();
			}
		}
	}

	const float SplineLength = CourseSpline->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float PlaceDistance = FMath::Clamp(
		SplineLength - FMath::Max(0.0f, FinishLineDistanceFromEnd),
		0.0f,
		SplineLength);

	FVector Position = CourseSpline->GetLocationAtDistanceAlongSpline(PlaceDistance, ESplineCoordinateSpace::World);
	FVector Direction = CourseSpline->GetDirectionAtDistanceAlongSpline(PlaceDistance, ESplineCoordinateSpace::World).GetSafeNormal();
	FVector Up = CourseSpline->GetUpVectorAtDistanceAlongSpline(PlaceDistance, ESplineCoordinateSpace::World).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
	if (Up.IsNearlyZero())
	{
		Up = FVector::UpVector;
	}

	// Snap finish line base to tagged terrain directly under the endpoint sample.
	{
		const FVector TraceStart = Position + FVector::UpVector * 5000.0f;
		const FVector TraceEnd = Position - FVector::UpVector * 50000.0f;
		FHitResult TerrainHit;
		if (FindPowderTerrainHit(World, this, TraceStart, TraceEnd, TerrainHit))
		{
			Position = TerrainHit.ImpactPoint;
			const FVector HitNormal = TerrainHit.ImpactNormal.GetSafeNormal();
			if (!HitNormal.IsNearlyZero())
			{
				Up = HitNormal;
			}
		}
	}

	Position += Up * FinishLineHeightOffset;
	const FRotator Rotation = Direction.Rotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APowderFinishLine* FinishLine = World->SpawnActor<APowderFinishLine>(SpawnClass, Position, Rotation, Params);
	if (FinishLine)
	{
		FinishLine->InitExtent(FMath::Max(100.0f, FinishLineWidth));
	}
#endif
}

void APowderCoursePath::GenerateBoundaryTrees()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CourseSpline->GetNumberOfSplinePoints() < 2)
	{
		return;
	}

	TArray<UStaticMesh*> ValidMeshes;
	ValidMeshes.Reserve(BoundaryTreeMeshes.Num());
	for (UStaticMesh* Mesh : BoundaryTreeMeshes)
	{
		if (Mesh)
		{
			ValidMeshes.Add(Mesh);
		}
	}

	if (ValidMeshes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowderCoursePath: GenerateBoundaryTrees aborted. No BoundaryTreeMeshes assigned."));
		return;
	}

	Modify();
	if (bReplaceExistingBoundaryTrees)
	{
		ClearBoundaryTrees();
	}

	BoundaryTreeHISMComponents.Reset();
	BoundaryTreeHISMComponents.Reserve(ValidMeshes.Num());

	for (int32 MeshIdx = 0; MeshIdx < ValidMeshes.Num(); ++MeshIdx)
	{
		UStaticMesh* Mesh = ValidMeshes[MeshIdx];
		if (!Mesh)
		{
			continue;
		}

		const FName ComponentName = MakeUniqueObjectName(
			this,
			UHierarchicalInstancedStaticMeshComponent::StaticClass(),
			FName(*FString::Printf(TEXT("BoundaryTrees_%d"), MeshIdx)));
		UHierarchicalInstancedStaticMeshComponent* HISM = NewObject<UHierarchicalInstancedStaticMeshComponent>(
			this,
			ComponentName);
		if (!HISM)
		{
			continue;
		}

		HISM->SetupAttachment(Root);
		const EComponentMobility::Type ParentMobility = Root ? Root->Mobility.GetValue() : EComponentMobility::Static;
		HISM->SetMobility(ParentMobility);
		HISM->SetStaticMesh(Mesh);
		if (BoundaryTreeMaterialOverride)
		{
			HISM->SetMaterial(0, BoundaryTreeMaterialOverride);
		}
		HISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HISM->SetCollisionObjectType(ECC_WorldStatic);
		HISM->SetCollisionResponseToAllChannels(ECR_Ignore);
		HISM->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		HISM->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		HISM->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		HISM->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		HISM->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
		HISM->ComponentTags.AddUnique(PowderObstacleTag);
		HISM->ComponentTags.AddUnique(PowderGeneratedTag);
		HISM->ComponentTags.AddUnique(PowderBoundaryTag);

		AddInstanceComponent(HISM);
		HISM->RegisterComponent();
		BoundaryTreeHISMComponents.Add(HISM);
	}

	if (BoundaryTreeHISMComponents.Num() == 0)
	{
		return;
	}

	FRandomStream RNG(BoundarySeed);
	const float Length = CourseSpline->GetSplineLength();
	const float Step = FMath::Max(100.0f, BoundaryTreeSpacing);
	const float EndDistance = Length + KINDA_SMALL_NUMBER;
	const float EdgeOffset = FMath::Max(100.0f, BoundaryEdgeOffset);

	int32 PlacedInstances = 0;
	for (float Dist = 0.0f; Dist <= EndDistance; Dist += Step)
	{
		const float ClampedDist = FMath::Min(Dist, Length);
		FVector Center = CourseSpline->GetLocationAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World);
		FVector Tangent = CourseSpline->GetDirectionAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World).GetSafeNormal();
		FVector Up = CourseSpline->GetUpVectorAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World).GetSafeNormal();
		if (Tangent.IsNearlyZero())
		{
			Tangent = FVector::ForwardVector;
		}
		if (Up.IsNearlyZero())
		{
			Up = FVector::UpVector;
		}
		FVector Right = FVector::CrossProduct(Up, Tangent).GetSafeNormal();
		if (Right.IsNearlyZero())
		{
			Right = FVector::CrossProduct(FVector::UpVector, Tangent).GetSafeNormal();
		}
		if (Right.IsNearlyZero())
		{
			continue;
		}

		for (int32 SideIdx = 0; SideIdx < 2; ++SideIdx)
		{
			const float SideSign = (SideIdx == 0) ? -1.0f : 1.0f;
			const float AlongOffset = RNG.FRandRange(-BoundaryAlongJitter, BoundaryAlongJitter);
			const float LateralOffset = EdgeOffset + RNG.FRandRange(-BoundaryLateralJitter, BoundaryLateralJitter);
			const FVector Candidate = Center + Tangent * AlongOffset + Right * (SideSign * LateralOffset);

			const FVector TraceStart = Candidate + FVector::UpVector * FMath::Max(100.0f, BoundaryTraceUp);
			const FVector TraceEnd = Candidate - FVector::UpVector * FMath::Max(1000.0f, BoundaryTraceDown);
			FHitResult TerrainHit;
			if (!FindPowderTerrainHit(World, this, TraceStart, TraceEnd, TerrainHit))
			{
				continue;
			}

			if (TerrainHit.ImpactNormal.Z < BoundaryMinNormalZ)
			{
				continue;
			}

			const FVector Pos = TerrainHit.ImpactPoint + TerrainHit.ImpactNormal.GetSafeNormal() * BoundaryHeightOffset;
			const float Yaw = Tangent.Rotation().Yaw + RNG.FRandRange(-BoundaryYawJitter, BoundaryYawJitter);
			const FTransform InstanceXform(FRotator(0.0f, Yaw, 0.0f), Pos, FVector::OneVector);

			const int32 HISMIdx = RNG.RandRange(0, BoundaryTreeHISMComponents.Num() - 1);
			if (UHierarchicalInstancedStaticMeshComponent* TargetHISM = BoundaryTreeHISMComponents[HISMIdx])
			{
				TargetHISM->AddInstanceWorldSpace(InstanceXform);
				++PlacedInstances;
			}
		}
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("PowderCoursePath: Generated %d boundary trees across %d HISM groups."),
		PlacedInstances,
		BoundaryTreeHISMComponents.Num());

	MarkPackageDirty();
#endif
}

void APowderCoursePath::GenerateWeatherZones()
{
#if WITH_EDITOR
	if (!CourseSpline)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CourseSpline->GetNumberOfSplinePoints() < 2)
	{
		return;
	}

	const int32 ZoneCount = FMath::Max(1, WeatherZoneCount);
	if (WeatherZonePresets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PowderCoursePath: GenerateWeatherZones aborted. No WeatherZonePresets assigned."));
		return;
	}

	// Optionally remove previously generated weather volumes
	if (bReplaceExistingWeatherZones)
	{
		TArray<AActor*> ToDestroy;
		for (TActorIterator<APowderWeatherVolume> It(World); It; ++It)
		{
			if ((*It)->Tags.Contains(PowderWeatherZoneTag))
			{
				ToDestroy.Add(*It);
			}
		}
		for (AActor* Actor : ToDestroy)
		{
			Actor->Destroy();
		}
	}

	const float SplineLength = CourseSpline->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float SegmentLength = SplineLength / static_cast<float>(ZoneCount);
	int32 PlacedCount = 0;

	for (int32 Idx = 0; Idx < ZoneCount; ++Idx)
	{
		const float StartDist = static_cast<float>(Idx) * SegmentLength;
		const float EndDist = FMath::Min(StartDist + SegmentLength, SplineLength);
		const float MidDist = (StartDist + EndDist) * 0.5f;

		const FVector StartPos = CourseSpline->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::World);
		const FVector EndPos = CourseSpline->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::World);
		const FVector MidPos = CourseSpline->GetLocationAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World);

		// Volume center at the midpoint of this segment
		const FVector Center = MidPos;

		// Compute extent: half the segment length along X, lateral width along Y, generous height
		const float HalfLength = FVector::Dist(StartPos, EndPos) * 0.5f;
		const float HalfWidth = FMath::Max(500.0f, WeatherZoneLateralWidth);
		const float HalfHeight = 1200.0f;

		// Orient the volume along the spline tangent at the midpoint
		const FVector Tangent = CourseSpline->GetDirectionAtDistanceAlongSpline(MidDist, ESplineCoordinateSpace::World).GetSafeNormal();
		const FRotator VolumeRotation = Tangent.IsNearlyZero() ? FRotator::ZeroRotator : Tangent.Rotation();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		APowderWeatherVolume* Volume = World->SpawnActor<APowderWeatherVolume>(
			APowderWeatherVolume::StaticClass(), Center, VolumeRotation, Params);
		if (!Volume)
		{
			continue;
		}

		Volume->Tags.AddUnique(PowderGeneratedTag);
		Volume->Tags.AddUnique(PowderWeatherZoneTag);
		Volume->SetActorLabel(FString::Printf(TEXT("WeatherZone_%d"), Idx));

		// Size the box to cover this segment
		if (Volume->VolumeBounds)
		{
			Volume->VolumeBounds->SetBoxExtent(FVector(HalfLength, HalfWidth, HalfHeight));
		}

		// Assign weather preset (wraps around if fewer presets than zones)
		const EWeatherPreset Preset = WeatherZonePresets[Idx % WeatherZonePresets.Num()];
		Volume->Config = UPowderWeatherManager::GetDefaultConfig(Preset);
		Volume->Priority = Idx;

		++PlacedCount;
	}

	UE_LOG(LogTemp, Log, TEXT("PowderCoursePath: Generated %d weather zones along spline."), PlacedCount);
#endif
}

void APowderCoursePath::ClearBoundaryTrees()
{
#if WITH_EDITOR
	Modify();

	TArray<UActorComponent*> ComponentsToRemove;
	GetComponents(UHierarchicalInstancedStaticMeshComponent::StaticClass(), ComponentsToRemove);
	for (UActorComponent* Comp : ComponentsToRemove)
	{
		UHierarchicalInstancedStaticMeshComponent* HISM = Cast<UHierarchicalInstancedStaticMeshComponent>(Comp);
		if (!HISM)
		{
			continue;
		}

		if (!HISM->ComponentHasTag(PowderBoundaryTag))
		{
			continue;
		}

		RemoveInstanceComponent(HISM);
		HISM->DestroyComponent();
	}

	BoundaryTreeHISMComponents.Reset();
	MarkPackageDirty();
#endif
}
