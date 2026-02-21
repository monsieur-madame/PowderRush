#include "Terrain/TerrainManager.h"

#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Terrain/PowderCoursePath.h"

namespace
{
TArray<float> GetDefaultWeatherBreakpoints()
{
	return { 0.25f, 0.55f, 0.80f };
}

TArray<float> SanitizeWeatherBreakpoints(const TArray<float>& InBreakpoints)
{
	TArray<float> Result;
	if (InBreakpoints.Num() < 3)
	{
		return GetDefaultWeatherBreakpoints();
	}

	Result.Reserve(3);
	Result.Add(FMath::Clamp(InBreakpoints[0], 0.0f, 1.0f));
	Result.Add(FMath::Clamp(InBreakpoints[1], Result[0], 1.0f));
	Result.Add(FMath::Clamp(InBreakpoints[2], Result[1], 1.0f));
	return Result;
}
} // namespace

ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	DefaultSurfaceProperties = FSurfaceProperties::GetPreset(ESurfaceType::Powder);
}

void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitializeCourseOnBeginPlay)
	{
		InitializeCourse();
	}
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdatePlayerDistance();
}

bool ATerrainManager::InitializeCourse()
{
	ClearRuntimeComponents();

	RuntimeSplineSamples.Reset();
	CachedWeatherBreakpointsNormalized.Reset();
	CachedCourseLength = 0.0f;
	PlayerDistance = 0.0f;
	LastProjectedSegmentIndex = INDEX_NONE;

	if (!BuildRuntimeSamplesFromCoursePathSpline(FindFirstUsableCoursePath()))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("TerrainManager: InitializeCourse failed. Place APowderCoursePath with >=2 spline points in the level."));
		return false;
	}

	return RuntimeSplineSamples.Num() >= 2;
}

void ATerrainManager::ResetTerrain()
{
	InitializeCourse();
}

void ATerrainManager::ClearRuntimeComponents()
{
	LastProjectedSegmentIndex = INDEX_NONE;
}

APowderCoursePath* ATerrainManager::FindFirstUsableCoursePath() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<APowderCoursePath> It(World); It; ++It)
	{
		APowderCoursePath* Candidate = *It;
		if (!Candidate || !Candidate->CourseSpline)
		{
			continue;
		}

		if (Candidate->CourseSpline->GetNumberOfSplinePoints() >= 2)
		{
			return Candidate;
		}
	}

	return nullptr;
}

bool ATerrainManager::BuildRuntimeSamplesFromCoursePathSpline(APowderCoursePath* PreferredPath)
{
	APowderCoursePath* SourcePath = PreferredPath;
	if (!SourcePath || !SourcePath->CourseSpline || SourcePath->CourseSpline->GetNumberOfSplinePoints() < 2)
	{
		SourcePath = FindFirstUsableCoursePath();
	}

	if (!SourcePath || !SourcePath->CourseSpline)
	{
		return false;
	}

	USplineComponent* Spline = SourcePath->CourseSpline;
	const float SplineLength = Spline->GetSplineLength();
	if (SplineLength <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float Step = FMath::Max(CoursePathSampleStep, 100.0f);
	RuntimeSplineSamples.Reset();
	RuntimeSplineSamples.Reserve(FMath::CeilToInt(SplineLength / Step) + 2);

	for (float Dist = 0.0f; Dist <= SplineLength + KINDA_SMALL_NUMBER; Dist += Step)
	{
		const float ClampedDist = FMath::Min(Dist, SplineLength);
		FRuntimeSplineSample Sample;
		Sample.Distance = ClampedDist;
		Sample.Position = Spline->GetLocationAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World);
		Sample.Tangent = Spline->GetDirectionAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World).GetSafeNormal();
		if (Sample.Tangent.IsNearlyZero())
		{
			Sample.Tangent = FVector::ForwardVector;
		}
		Sample.UpVector = Spline->GetUpVectorAtDistanceAlongSpline(ClampedDist, ESplineCoordinateSpace::World).GetSafeNormal();
		if (Sample.UpVector.IsNearlyZero())
		{
			Sample.UpVector = FVector::UpVector;
		}
		RuntimeSplineSamples.Add(Sample);
	}

	if (RuntimeSplineSamples.Num() == 0
		|| RuntimeSplineSamples.Last().Distance < SplineLength - KINDA_SMALL_NUMBER)
	{
		FRuntimeSplineSample EndSample;
		EndSample.Distance = SplineLength;
		EndSample.Position = Spline->GetLocationAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World);
		EndSample.Tangent = Spline->GetDirectionAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World).GetSafeNormal();
		if (EndSample.Tangent.IsNearlyZero())
		{
			EndSample.Tangent = FVector::ForwardVector;
		}
		EndSample.UpVector = Spline->GetUpVectorAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World).GetSafeNormal();
		if (EndSample.UpVector.IsNearlyZero())
		{
			EndSample.UpVector = FVector::UpVector;
		}
		RuntimeSplineSamples.Add(EndSample);
	}

	StartDistanceOffset = FMath::Max(0.0f, SourcePath->StartDistanceOffset);
	RespawnBacktrackDistance = FMath::Max(0.0f, SourcePath->RespawnBacktrackDistance);
	CachedCourseLength = SplineLength;
	CachedWeatherBreakpointsNormalized = SanitizeWeatherBreakpoints(SourcePath->WeatherBreakpointsNormalized);
	DefaultSurfaceProperties = FSurfaceProperties::GetPreset(ESurfaceType::Powder);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("TerrainManager: Initialized from APowderCoursePath '%s' (Course=%s, Samples=%d, StartOffset=%.1f, RespawnBacktrack=%.1f)."),
		*SourcePath->GetName(),
		*SourcePath->CourseName.ToString(),
		RuntimeSplineSamples.Num(),
		StartDistanceOffset,
		RespawnBacktrackDistance);

	return RuntimeSplineSamples.Num() >= 2;
}

void ATerrainManager::UpdatePlayerDistance()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	APawn* Player = PC ? PC->GetPawn() : nullptr;
	if (!Player)
	{
		return;
	}

	PlayerDistance = ProjectPositionOntoCourse(Player->GetActorLocation());
}

float ATerrainManager::ProjectPositionOntoCourse(const FVector& WorldPos) const
{
	if (RuntimeSplineSamples.Num() == 0)
	{
		return 0.0f;
	}

	if (RuntimeSplineSamples.Num() == 1)
	{
		return RuntimeSplineSamples[0].Distance;
	}

	const int32 SegmentCount = RuntimeSplineSamples.Num() - 1;
	float BestDistSq = TNumericLimits<float>::Max();
	float BestDistance = 0.0f;
	int32 BestSegment = INDEX_NONE;

	auto EvaluateSegment = [&](int32 Idx)
	{
		if (Idx < 0 || Idx >= SegmentCount)
		{
			return;
		}

		const FRuntimeSplineSample& A = RuntimeSplineSamples[Idx];
		const FRuntimeSplineSample& B = RuntimeSplineSamples[Idx + 1];

		const FVector AB = B.Position - A.Position;
		const float ABSizeSq = FMath::Max(AB.SizeSquared(), KINDA_SMALL_NUMBER);
		const float T = FMath::Clamp(FVector::DotProduct(WorldPos - A.Position, AB) / ABSizeSq, 0.0f, 1.0f);
		const FVector Closest = A.Position + AB * T;
		const float DistSq = FVector::DistSquared(WorldPos, Closest);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestDistance = FMath::Lerp(A.Distance, B.Distance, T);
			BestSegment = Idx;
		}
	};

	constexpr int32 HintWindow = 6;
	constexpr float HintFallbackDistance = 5000.0f;
	const float HintFallbackDistanceSq = HintFallbackDistance * HintFallbackDistance;

	if (LastProjectedSegmentIndex != INDEX_NONE)
	{
		const int32 ClampedHint = FMath::Clamp(LastProjectedSegmentIndex, 0, SegmentCount - 1);
		for (int32 Idx = ClampedHint - HintWindow; Idx <= ClampedHint + HintWindow; ++Idx)
		{
			EvaluateSegment(Idx);
		}
	}

	if (BestSegment == INDEX_NONE || BestDistSq > HintFallbackDistanceSq)
	{
		const int32 CoarseStep = FMath::Max(1, SegmentCount / 64);
		for (int32 Idx = 0; Idx < SegmentCount; Idx += CoarseStep)
		{
			EvaluateSegment(Idx);
		}
		EvaluateSegment(SegmentCount - 1);

		if (BestSegment != INDEX_NONE)
		{
			const int32 LocalWindow = FMath::Max(HintWindow, CoarseStep * 2);
			for (int32 Idx = BestSegment - LocalWindow; Idx <= BestSegment + LocalWindow; ++Idx)
			{
				EvaluateSegment(Idx);
			}
		}
	}

	UpdateProjectionHint(BestSegment);
	return FMath::Clamp(BestDistance, 0.0f, CachedCourseLength);
}

FVector ATerrainManager::GetPositionAtDistance(float Distance) const
{
	if (RuntimeSplineSamples.Num() == 0)
	{
		return GetActorLocation();
	}

	if (RuntimeSplineSamples.Num() == 1)
	{
		return RuntimeSplineSamples[0].Position;
	}

	const float ClampedDistance = FMath::Clamp(Distance, 0.0f, CachedCourseLength);
	if (ClampedDistance <= RuntimeSplineSamples[0].Distance)
	{
		return RuntimeSplineSamples[0].Position;
	}
	if (ClampedDistance >= RuntimeSplineSamples.Last().Distance)
	{
		return RuntimeSplineSamples.Last().Position;
	}

	const int32 Idx = FindSegmentIndexForDistance(ClampedDistance);
	if (Idx != INDEX_NONE)
	{
		const FRuntimeSplineSample& A = RuntimeSplineSamples[Idx];
		const FRuntimeSplineSample& B = RuntimeSplineSamples[Idx + 1];
		const float SegmentLen = FMath::Max(B.Distance - A.Distance, KINDA_SMALL_NUMBER);
		const float T = FMath::Clamp((ClampedDistance - A.Distance) / SegmentLen, 0.0f, 1.0f);
		return FMath::Lerp(A.Position, B.Position, T);
	}

	return RuntimeSplineSamples.Last().Position;
}

FVector ATerrainManager::GetDirectionAtDistance(float Distance) const
{
	if (RuntimeSplineSamples.Num() == 0)
	{
		return FVector::ForwardVector;
	}

	if (RuntimeSplineSamples.Num() == 1)
	{
		return RuntimeSplineSamples[0].Tangent.GetSafeNormal();
	}

	const float ClampedDistance = FMath::Clamp(Distance, 0.0f, CachedCourseLength);
	const int32 Idx = FindSegmentIndexForDistance(ClampedDistance);
	if (Idx != INDEX_NONE)
	{
		const FRuntimeSplineSample& A = RuntimeSplineSamples[Idx];
		const FRuntimeSplineSample& B = RuntimeSplineSamples[Idx + 1];
		const float SegmentLen = FMath::Max(B.Distance - A.Distance, KINDA_SMALL_NUMBER);
		const float T = FMath::Clamp((ClampedDistance - A.Distance) / SegmentLen, 0.0f, 1.0f);
		return FMath::Lerp(A.Tangent, B.Tangent, T).GetSafeNormal();
	}

	return RuntimeSplineSamples.Last().Tangent.GetSafeNormal();
}

FVector ATerrainManager::GetSlopeStartPosition() const
{
	if (RuntimeSplineSamples.Num() == 0)
	{
		return GetActorLocation() + FVector(0.0f, 0.0f, SpawnHeightOffset);
	}

	const float StartDistance = FMath::Clamp(StartDistanceOffset, 0.0f, CachedCourseLength);
	const FVector BasePos = GetPositionAtDistance(StartDistance);

	FVector Up = FVector::UpVector;
	const int32 SegmentIdx = FindSegmentIndexForDistance(StartDistance);
	if (SegmentIdx != INDEX_NONE)
	{
		Up = RuntimeSplineSamples[SegmentIdx].UpVector;
	}
	if (Up.IsNearlyZero())
	{
		Up = FVector::UpVector;
	}

	return BasePos + Up.GetSafeNormal() * SpawnHeightOffset;
}

FVector ATerrainManager::GetSlopeDownhill() const
{
	return GetDirectionAtDistance(PlayerDistance);
}

FVector ATerrainManager::GetStartDownhill() const
{
	const float StartDistance = FMath::Clamp(StartDistanceOffset, 0.0f, CachedCourseLength);
	FVector Direction = GetDirectionAtDistance(StartDistance).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = FVector::ForwardVector;
	}
	return Direction;
}

FRotator ATerrainManager::GetStartFacingRotation() const
{
	return BuildFacingRotationFromDirection(GetStartDownhill());
}

FVector ATerrainManager::GetRespawnPosition() const
{
	const float RespawnDistance = FMath::Clamp(
		PlayerDistance - RespawnBacktrackDistance,
		0.0f,
		CachedCourseLength);

	const FVector BasePos = GetPositionAtDistance(RespawnDistance);

	FVector Up = FVector::UpVector;
	const int32 SegmentIdx = FindSegmentIndexForDistance(RespawnDistance);
	if (SegmentIdx != INDEX_NONE)
	{
		Up = RuntimeSplineSamples[SegmentIdx].UpVector;
	}
	if (Up.IsNearlyZero())
	{
		Up = FVector::UpVector;
	}

	return BasePos + Up.GetSafeNormal() * SpawnHeightOffset;
}

FRotator ATerrainManager::GetRespawnFacingRotation() const
{
	const float RespawnDistance = FMath::Clamp(
		PlayerDistance - RespawnBacktrackDistance,
		0.0f,
		CachedCourseLength);
	FVector Direction = GetDirectionAtDistance(RespawnDistance).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetStartDownhill();
	}
	return BuildFacingRotationFromDirection(Direction);
}

const FSurfaceProperties* ATerrainManager::GetCurrentSurfaceProperties() const
{
	return GetSurfacePropertiesAtDistance(PlayerDistance);
}

const FSurfaceProperties* ATerrainManager::GetSurfacePropertiesAtDistance(float Distance) const
{
	(void)Distance;
	return &DefaultSurfaceProperties;
}

bool ATerrainManager::SampleSurfaceAtWorldPosition(
	const FVector& WorldPos,
	FSurfaceProperties& OutSurface,
	float& OutCourseDistance) const
{
	OutCourseDistance = ProjectPositionOntoCourse(WorldPos);
	if (const FSurfaceProperties* Surface = GetSurfacePropertiesAtDistance(OutCourseDistance))
	{
		OutSurface = *Surface;
		return true;
	}

	OutSurface = DefaultSurfaceProperties;
	return false;
}

bool ATerrainManager::SampleCourseFrameAtWorldPosition(
	const FVector& WorldPos,
	FVector& OutTangent,
	FVector& OutUp,
	float& OutCourseDistance,
	float& OutCrossTrackDistance) const
{
	OutCourseDistance = ProjectPositionOntoCourse(WorldPos);
	OutTangent = GetDirectionAtDistance(OutCourseDistance).GetSafeNormal();
	OutUp = GetUpVectorAtDistance(OutCourseDistance).GetSafeNormal();

	if (OutTangent.IsNearlyZero())
	{
		OutTangent = FVector::ForwardVector;
	}
	if (OutUp.IsNearlyZero())
	{
		OutUp = FVector::UpVector;
	}

	const FVector CoursePos = GetPositionAtDistance(OutCourseDistance);
	const FVector CourseRight = FVector::CrossProduct(OutUp, OutTangent).GetSafeNormal();
	OutCrossTrackDistance = CourseRight.IsNearlyZero() ? 0.0f : FVector::DotProduct(WorldPos - CoursePos, CourseRight);
	return RuntimeSplineSamples.Num() >= 2;
}

int32 ATerrainManager::FindSegmentIndexForDistance(float Distance) const
{
	if (RuntimeSplineSamples.Num() < 2)
	{
		return INDEX_NONE;
	}

	const float ClampedDistance = FMath::Clamp(
		Distance,
		RuntimeSplineSamples[0].Distance,
		RuntimeSplineSamples.Last().Distance);
	int32 Low = 0;
	int32 High = RuntimeSplineSamples.Num() - 2;
	while (Low <= High)
	{
		const int32 Mid = Low + (High - Low) / 2;
		const float StartDistance = RuntimeSplineSamples[Mid].Distance;
		const float EndDistance = RuntimeSplineSamples[Mid + 1].Distance;
		if (ClampedDistance < StartDistance)
		{
			High = Mid - 1;
		}
		else if (ClampedDistance > EndDistance)
		{
			Low = Mid + 1;
		}
		else
		{
			return Mid;
		}
	}

	return FMath::Clamp(Low, 0, RuntimeSplineSamples.Num() - 2);
}

FVector ATerrainManager::GetUpVectorAtDistance(float Distance) const
{
	if (RuntimeSplineSamples.Num() == 0)
	{
		return FVector::UpVector;
	}

	if (RuntimeSplineSamples.Num() == 1)
	{
		return RuntimeSplineSamples[0].UpVector.GetSafeNormal();
	}

	const float ClampedDistance = FMath::Clamp(Distance, 0.0f, CachedCourseLength);
	const int32 Idx = FindSegmentIndexForDistance(ClampedDistance);
	if (Idx != INDEX_NONE)
	{
		const FRuntimeSplineSample& A = RuntimeSplineSamples[Idx];
		const FRuntimeSplineSample& B = RuntimeSplineSamples[Idx + 1];
		const float SegmentLen = FMath::Max(B.Distance - A.Distance, KINDA_SMALL_NUMBER);
		const float T = FMath::Clamp((ClampedDistance - A.Distance) / SegmentLen, 0.0f, 1.0f);
		return FMath::Lerp(A.UpVector, B.UpVector, T).GetSafeNormal();
	}

	return RuntimeSplineSamples.Last().UpVector.GetSafeNormal();
}

void ATerrainManager::UpdateProjectionHint(int32 SegmentIndex) const
{
	if (RuntimeSplineSamples.Num() < 2 || SegmentIndex == INDEX_NONE)
	{
		LastProjectedSegmentIndex = INDEX_NONE;
		return;
	}

	LastProjectedSegmentIndex = FMath::Clamp(SegmentIndex, 0, RuntimeSplineSamples.Num() - 2);
}

FRotator ATerrainManager::BuildFacingRotationFromDirection(const FVector& Direction) const
{
	FVector SafeDirection = Direction.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		SafeDirection = FVector::ForwardVector;
	}

	FRotator Facing = SafeDirection.Rotation();
	Facing.Pitch = 0.0f;
	Facing.Roll = 0.0f;
	Facing.Yaw += SpawnFacingYawOffset;
	return Facing;
}
