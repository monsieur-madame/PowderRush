#include "Player/PowderMovementComponent.h"
#include "Terrain/PowderJump.h"
#include "Terrain/PowderSurfaceQueryProvider.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

UPowderMovementComponent::UPowderMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UPowderMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentSpeed = 0.0f;
	BoostMeter = 0.0f;
	CurrentCarveAngle = 0.0f;
	LastTurnRateDegPerSec = 0.0f;
	GroundNormalStability = 1.0f;
	CurrentSurface = FSurfaceProperties::GetPreset(ESurfaceType::Powder);
	ResolveSurfaceQueryProvider(0.0f);
}

void UPowderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdatedComponent || ShouldSkipUpdate(DeltaTime) || bIsFrozen)
	{
		return;
	}

	if (bIsAirborne)
	{
		UpdateAirborne(DeltaTime);
		return;
	}

	// Tick landing blend
	if (bIsLandingBlend)
	{
		LandingBlendTimer -= DeltaTime;
		if (LandingBlendTimer <= 0.0f)
		{
			LandingBlendTimer = 0.0f;
			bIsLandingBlend = false;
		}
	}

	TickTuningBlend(DeltaTime);
	TickSurfaceBlend(DeltaTime);
	UpdateTerrainFollowing(DeltaTime);
	UpdateCarving(DeltaTime);
	UpdateSpeed(DeltaTime);
	UpdateBoost(DeltaTime);
	ApplyMovement(DeltaTime);
}

void UPowderMovementComponent::SetCarveInput(float Value)
{
	CarveInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UPowderMovementComponent::ReleaseCarve()
{
	CarveInput = 0.0f;
}

void UPowderMovementComponent::ActivateBoost()
{
	if (BoostMeter >= 1.0f)
	{
		bIsBoosting = true;
		BoostTimer = BoostDuration;
		BoostMeter = 0.0f;
		OnBoostActivated.Broadcast();
	}
}

void UPowderMovementComponent::TriggerSpeedBoost(float BurstSpeed, float Duration)
{
	bIsBoosting = true;
	BoostTimer = Duration;
	BoostBurstSpeed = BurstSpeed;
	OnBoostActivated.Broadcast();
}

float UPowderMovementComponent::GetSpeedNormalized() const
{
	return MaxSpeed > 0.0f ? FMath::Clamp(CurrentSpeed / MaxSpeed, 0.0f, 1.0f) : 0.0f;
}

void UPowderMovementComponent::UpdateTerrainFollowing(float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const float CapsuleHalfHeight = GetOwnerCapsuleHalfHeight();
	// Start trace from well above the character to avoid starting inside geometry after big air
	FVector Start = CurrentLocation + FVector::UpVector * 400.0f;
	FVector End = Start - FVector::UpVector * (400.0f + TerrainTraceDistance);

	FHitResult Hit;
	if (TraceForTaggedTerrain(Start, End, Hit)
		&& Hit.ImpactNormal.Z > MinGroundNormalZ)  // Only follow upward-facing surfaces — reject walls, edges, undersides
	{
		bOnGround = true;
		const FVector RawNormal = Hit.ImpactNormal.GetSafeNormal();
		const FVector PrevNormal = SlopeNormal.GetSafeNormal();
		const float NormalAgreement = FMath::Clamp(FVector::DotProduct(PrevNormal, RawNormal), -1.0f, 1.0f);
		const float StabilityTarget = FMath::GetMappedRangeValueClamped(
			FVector2D(-1.0f, 1.0f),
			FVector2D(0.0f, 1.0f),
			NormalAgreement);
		GroundNormalStability = FMath::FInterpTo(GroundNormalStability, StabilityTarget, DeltaTime, 8.0f);

		SlopeNormal = FMath::VInterpTo(
			SlopeNormal,
			RawNormal,
			DeltaTime,
			FMath::Max(0.0f, GroundNormalFilterSpeed)).GetSafeNormal();
		if (SlopeNormal.IsNearlyZero())
		{
			SlopeNormal = RawNormal.IsNearlyZero() ? FVector::UpVector : RawNormal;
		}

		// Derive downhill direction: course spline tangent is authoritative when available,
		// gravity projection is fallback only (degenerate on flat terrain).
		bool bHasCourseDirection = false;
		if (IPowderSurfaceQueryProvider* SurfaceProvider = ResolveSurfaceQueryProvider(DeltaTime))
		{
			FVector CourseTangent = FVector::ForwardVector;
			FVector CourseUp = FVector::UpVector;
			float CourseDistance = 0.0f;
			float CrossTrackDistance = 0.0f;
			if (SurfaceProvider->SampleCourseFrameAtWorldPosition(
				Hit.ImpactPoint,
				CourseTangent,
				CourseUp,
				CourseDistance,
				CrossTrackDistance))
			{
				FVector CourseOnSlope = FVector::VectorPlaneProject(CourseTangent.GetSafeNormal(), SlopeNormal).GetSafeNormal();
				if (CourseOnSlope.IsNearlyZero())
				{
					CourseOnSlope = CourseTangent.GetSafeNormal();
				}
				if (!CourseOnSlope.IsNearlyZero())
				{
					// Direct assignment — no blend, no interp lag
					SlopeForward = CourseOnSlope;
					bHasCourseDirection = true;
				}
			}
		}

		// Fallback: gravity-downhill when no course is available
		if (!bHasCourseDirection)
		{
			const FVector Gravity = FVector(0.0f, 0.0f, -1.0f);
			const FVector SlopeDownhill = FVector::VectorPlaneProject(Gravity, SlopeNormal).GetSafeNormal();
			if (!SlopeDownhill.IsNearlyZero())
			{
				if (!bHeadingInitialized)
				{
					SlopeForward = SlopeDownhill;
				}
				else
				{
					SlopeForward = FMath::VInterpTo(
						SlopeForward,
						SlopeDownhill,
						DeltaTime,
						FMath::Max(0.0f, SlopeForwardInterpSpeed)).GetSafeNormal();
				}
			}
		}

		// Gravity-based terrain following: only snap UP, let gravity handle downward
		float DesiredHeight = Hit.ImpactPoint.Z + CapsuleHalfHeight + TerrainContactOffset;
		float HeightDelta = DesiredHeight - CurrentLocation.Z;

		if (HeightDelta > TerrainSnapThreshold)
		{
			// Terrain is above us (going uphill / riding up ramp) — snap up to stay on surface
			FVector HeightAdjust = FVector(0.0f, 0.0f, HeightDelta);
			UpdatedComponent->MoveComponent(HeightAdjust, UpdatedComponent->GetComponentRotation(), true);
			GroundVerticalVelocity = 0.0f;
		}
		else if (HeightDelta >= -TerrainSnapThreshold)
		{
			// Within snap threshold — solidly on ground
			GroundVerticalVelocity = 0.0f;
		}
		else
		{
			// Terrain is below us (drop, bump, ramp lip) — let gravity pull us down naturally
			// Inherit upward velocity from slope movement on first frame of separation
			if (GroundVerticalVelocity >= 0.0f && Velocity.Z > 0.0f)
			{
				GroundVerticalVelocity = Velocity.Z;
			}
			GroundVerticalVelocity -= GravityAcceleration * DeltaTime;

			float VerticalMove = GroundVerticalVelocity * DeltaTime;

			// Don't go below the terrain surface
			if (VerticalMove < HeightDelta)
			{
				VerticalMove = HeightDelta;
				GroundVerticalVelocity = 0.0f; // Settled back on terrain
			}

			UpdatedComponent->MoveComponent(
				FVector(0.0f, 0.0f, VerticalMove),
				UpdatedComponent->GetComponentRotation(), true);

			// If separated enough, transition to full airborne physics
			float Separation = (CurrentLocation.Z + VerticalMove) - DesiredHeight;
			if (Separation > LedgeLaunchThreshold)
			{
				LaunchIntoAir(FVector(0.0f, 0.0f, GroundVerticalVelocity));
				GroundVerticalVelocity = 0.0f;
				return;
			}
		}

		// Initialize heading from slope on first ground contact
		if (!bHeadingInitialized && !SlopeForward.IsNearlyZero())
		{
			DesiredYaw = SlopeForward.Rotation().Yaw;
			VisualYaw = DesiredYaw;
			bHeadingInitialized = true;
		}

		if (IPowderSurfaceQueryProvider* SurfaceProvider = ResolveSurfaceQueryProvider(DeltaTime))
		{
			FSurfaceProperties SampledSurface = CurrentSurface;
			float SampledDistance = 0.0f;
			if (SurfaceProvider->SampleSurfaceAtWorldPosition(Hit.ImpactPoint, SampledSurface, SampledDistance))
			{
				SetCurrentSurface(SampledSurface);
			}
		}
	}
	else
	{
		// No valid terrain hit — go airborne with current velocity
		bOnGround = false;
		GroundNormalStability = FMath::FInterpTo(GroundNormalStability, 0.0f, DeltaTime, 4.0f);

		if (CurrentSpeed > 50.0f)
		{
			// Moving with speed — transition to full airborne physics
			float LaunchZ = FMath::Max(GroundVerticalVelocity, Velocity.Z);
			LaunchIntoAir(FVector(0.0f, 0.0f, FMath::Max(0.0f, LaunchZ)));
			GroundVerticalVelocity = 0.0f;
			return;
		}

		// Stationary / very slow — just apply gravity to settle
		FVector GravityDrop = FVector(0.0f, 0.0f, -GravityAcceleration * DeltaTime);
		UpdatedComponent->MoveComponent(GravityDrop, UpdatedComponent->GetComponentRotation(), true);
	}
}

void UPowderMovementComponent::UpdateCarving(float DeltaTime)
{
	// --- Landing blend: reduced carve authority while settling ---
	float LandingControlMod = 1.0f;
	if (bIsLandingBlend && LandingBlendDuration > KINDA_SMALL_NUMBER)
	{
		float LandingAlpha = 1.0f - (LandingBlendTimer / LandingBlendDuration);
		LandingControlMod = LandingAlpha;
	}

	// --- Landing penalty tick (quality-based) ---
	if (LandingPenaltyTimer > 0.0f)
	{
		LandingPenaltyTimer -= DeltaTime;
		if (LandingPenaltyTimer > 0.0f)
		{
			LandingControlMod *= FMath::Lerp(1.0f, LandingControlPenaltyFactor, LandingPenaltyStrength);
		}
		else
		{
			LandingPenaltyTimer = 0.0f;
			LandingPenaltyStrength = 0.0f;
		}
	}

	// Smooth raw input for less jittery carving
	SmoothedCarveInput = FMath::FInterpTo(SmoothedCarveInput, CarveInput, DeltaTime, CarveInputSmoothing);

	// --- Progressive edge engagement ---
	const bool bHasInput = FMath::Abs(SmoothedCarveInput) > 0.05f;
	const float EdgeTarget = bHasInput ? 1.0f : 0.0f;
	const float EdgeRate = bHasInput ? EdgeEngageRate : EdgeDisengageRate;
	EdgeDepth = FMath::FInterpTo(EdgeDepth, EdgeTarget, DeltaTime, EdgeRate);
	float EffectiveEdge = bHasInput ? FMath::Max(EdgeMinDepth, EdgeDepth) : EdgeDepth;

	// --- Edge-to-edge transition (flat ski phase) ---
	int32 CurrentSign = (SmoothedCarveInput > 0.05f) ? 1 : (SmoothedCarveInput < -0.05f) ? -1 : 0;
	if (CurrentSign != 0 && LastCarveSign != 0 && CurrentSign != LastCarveSign)
	{
		bInEdgeTransition = true;
		EdgeTransitionTimer = EdgeTransitionTime;
		EdgeDepth = 0.0f;
	}
	if (CurrentSign != 0)
	{
		LastCarveSign = CurrentSign;
	}
	if (bInEdgeTransition)
	{
		EdgeTransitionTimer -= DeltaTime;
		EffectiveEdge *= EdgeTransitionGrip;
		if (EdgeTransitionTimer <= 0.0f)
		{
			bInEdgeTransition = false;
		}
	}

	float TargetAngle = SmoothedCarveInput * MaxCarveAngle * EffectiveEdge;

	// Speed-dependent angle limit
	float SpeedNorm = GetSpeedNormalized();
	float SpeedLimitedAngle = FMath::Lerp(MaxCarveAngle, MinTurnAngleAtMaxSpeed, SpeedNorm * SpeedTurnLimitFactor);
	TargetAngle = FMath::Clamp(TargetAngle, -SpeedLimitedAngle, SpeedLimitedAngle);

	// --- Turn exit commitment (Feature 4) ---
	if (bHasInput)
	{
		// Active input clears any commitment
		TurnCommitTimer = 0.0f;
		CommittedCarveAngle = 0.0f;
	}
	else
	{
		// Start commitment on release while carving (only once per release)
		if (FMath::Abs(CommittedCarveAngle) < 0.1f && TurnCommitTimer <= 0.0f
			&& FMath::Abs(CurrentCarveAngle) > 5.0f && TurnCommitTime > 0.0f)
		{
			CommittedCarveAngle = CurrentCarveAngle;
			// Depth-scaled commitment: deeper carves carry more momentum
			float CarveDepthAtRelease = FMath::Clamp(FMath::Abs(CurrentCarveAngle) / FMath::Max(1.0f, MaxCarveAngle), 0.0f, 1.0f);
			TurnCommitTimer = TurnCommitTime * (0.5f + 0.5f * CarveDepthAtRelease);
		}

		if (TurnCommitTimer > 0.0f)
		{
			// During commit window: override target with decaying committed angle
			// Deeper carves decay less (hold the arc longer)
			float CarveDepthAtRelease = FMath::Clamp(FMath::Abs(CommittedCarveAngle) / FMath::Max(1.0f, MaxCarveAngle), 0.0f, 1.0f);
			float ScaledCommitDecay = TurnCommitDecay * (1.0f - 0.5f * CarveDepthAtRelease);
			TurnCommitTimer -= DeltaTime;
			float ScaledCommitTime = TurnCommitTime * (0.5f + 0.5f * CarveDepthAtRelease);
			float CommitProgress = 1.0f - FMath::Clamp(TurnCommitTimer / FMath::Max(KINDA_SMALL_NUMBER, ScaledCommitTime), 0.0f, 1.0f);
			TargetAngle = FMath::Lerp(CommittedCarveAngle, 0.0f, CommitProgress * ScaledCommitDecay);
		}
	}

	float InterpSpeed = CarveRate * CurrentSurface.CarveGrip * LandingControlMod;

	// Use separate (slower) return rate when releasing input
	if (FMath::Abs(TargetAngle) < FMath::Abs(CurrentCarveAngle))
	{
		InterpSpeed = CarveReturnRate * CurrentSurface.CarveGrip * LandingControlMod;
		// During commitment, use even slower interp (30% of return rate)
		if (TurnCommitTimer > 0.0f)
		{
			InterpSpeed *= 0.3f;
		}
	}

	CurrentCarveAngle = FMath::FInterpTo(CurrentCarveAngle, TargetAngle, DeltaTime, InterpSpeed);

	// --- Carve pressure accumulator ---
	if (bHasInput && EdgeDepth > 0.5f)
	{
		CarvePressure = FMath::FInterpTo(CarvePressure, EdgeDepth, DeltaTime, CarvePressureBuildRate);
	}
	else
	{
		CarvePressure = FMath::FInterpTo(CarvePressure, 0.0f, DeltaTime, CarvePressureDecayRate);
	}

	// --- Speed-dependent turn rate ---
	const float SpeedAlpha = FMath::Pow(SpeedNorm, FMath::Max(0.01f, SpeedTurnRateExponent));
	const float BaseTurnRate = FMath::Lerp(TurnRateLimitDegPerSec, TurnRateLimitDegPerSec * SpeedTurnRateMin, SpeedAlpha) * LandingControlMod;
	const float EffectiveTurnRate = BaseTurnRate * (1.0f + CarvePressure * CarvePressureTurnBonus);
	const float EffectiveDownhillAlign = FMath::Lerp(DownhillAlignRate, DownhillAlignRate * SpeedTurnRateMin, SpeedAlpha);

	// Rate-limited heading update for more readable, committed carve arcs.
	const float DownhillYaw = SlopeForward.Rotation().Yaw;
	const float TargetYaw = DownhillYaw + CurrentCarveAngle;
	const float PrevYaw = DesiredYaw;
	const float TurnRate = FMath::Max(0.0f, EffectiveTurnRate);
	DesiredYaw = FMath::FixedTurn(DesiredYaw, TargetYaw, TurnRate * DeltaTime);

	// When input is neutral and not in commitment, settle toward downhill.
	if (FMath::Abs(CarveInput) < 0.1f && TurnCommitTimer <= 0.0f)
	{
		DesiredYaw = FMath::FixedTurn(
			DesiredYaw,
			DownhillYaw,
			FMath::Max(0.0f, EffectiveDownhillAlign) * DeltaTime);
	}

	LastTurnRateDegPerSec = (DeltaTime > KINDA_SMALL_NUMBER)
		? FMath::Abs(FMath::FindDeltaAngleDegrees(PrevYaw, DesiredYaw)) / DeltaTime
		: 0.0f;
}

void UPowderMovementComponent::UpdateSpeed(float DeltaTime)
{
	if (OllieCooldownTimer > 0.0f)
	{
		OllieCooldownTimer -= DeltaTime;
	}

	if (WipeoutRecoveryTimer > 0.0f)
	{
		WipeoutRecoveryTimer -= DeltaTime;
		return;
	}

	// Gravity-based acceleration along sampled terrain normal. Falls back to SlopeAngle if no ground.
	float EffectiveSlopeAngle = SlopeAngle;
	if (bOnGround)
	{
		const float CosAngle = FVector::DotProduct(SlopeNormal, FVector::UpVector);
		EffectiveSlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, 0.0f, 1.0f)));
	}
	const float SlopeAngleForAccel = FMath::Clamp(
		EffectiveSlopeAngle,
		0.0f,
		FMath::Max(1.0f, MaxAccelerationSlopeAngle));
	const float SlopeComponent = FMath::Sin(FMath::DegreesToRadians(SlopeAngleForAccel));
	const float GravityScale = FMath::Max(0.05f, GravityAlongSlopeScale);
	const float GravityForce = GravityAcceleration * GravityScale * SlopeComponent;

	// Surface friction
	float SurfaceFriction = BaseFriction * CurrentSurface.Friction;

	// --- Heading-relative speed dynamics (Feature 3) ---
	// How far off the fall line is our heading? 0 = downhill, 0.5 = traverse, 1.0 = uphill
	const float DownhillYaw = SlopeForward.Rotation().Yaw;
	const float HeadingDeltaDeg = FMath::Abs(FMath::FindDeltaAngleDegrees(DesiredYaw, DownhillYaw));
	const float HeadingAlpha = FMath::Clamp(HeadingDeltaDeg / 180.0f, 0.0f, 1.0f);

	// Piecewise gravity mod: 0-90 deg: 1.0→TraverseFactor, 90-180 deg: TraverseFactor→UphillFactor
	float HeadingGravityMod;
	if (HeadingAlpha <= 0.5f)
	{
		HeadingGravityMod = FMath::Lerp(1.0f, HeadingTraverseFactor, HeadingAlpha * 2.0f);
	}
	else
	{
		HeadingGravityMod = FMath::Lerp(HeadingTraverseFactor, HeadingUphillFactor, (HeadingAlpha - 0.5f) * 2.0f);
	}

	const float AdjustedGravityForce = GravityForce * HeadingGravityMod;

	// Friction reduction when off fall line (peaks at traverse, stays reduced uphill)
	const float HeadingFrictionMod = FMath::Lerp(1.0f, HeadingFrictionScale, FMath::Min(1.0f, HeadingAlpha * 2.0f));
	const float AdjustedFriction = SurfaceFriction * HeadingFrictionMod;

	// Carve speed bleed uses an exponential carve-depth curve for smoother low-angle carving.
	const float MaxCarveSafe = FMath::Max(1.0f, MaxCarveAngle);
	const float CarveDepth = FMath::Clamp(FMath::Abs(CurrentCarveAngle) / MaxCarveSafe, 0.0f, 1.0f);
	const float RawCarveBleed = CarveSpeedBleed * FMath::Pow(CarveDepth, FMath::Max(0.01f, CarveBleedExponent));
	SmoothedCarveBleed = FMath::FInterpTo(SmoothedCarveBleed, RawCarveBleed, DeltaTime, CarveBleedSmoothing);
	const float CarveBleed = SmoothedCarveBleed * (1.0f + CarvePressure * CarvePressureBleedBonus);

	// Apply equipment and surface modifiers
	float SpeedMod = EquipmentStats.SpeedMultiplier * CurrentSurface.SpeedMultiplier;

	// Calculate net acceleration (using heading-adjusted gravity and friction)
	float Acceleration = (AdjustedGravityForce - (AdjustedFriction + CarveBleed) * CurrentSpeed) * SpeedMod;
	CurrentSpeed += Acceleration * DeltaTime;
	CurrentSpeed = FMath::Clamp(CurrentSpeed, 0.0f, MaxSpeed * SpeedMod);
}

void UPowderMovementComponent::UpdateBoost(float DeltaTime)
{
	if (bIsBoosting)
	{
		BoostTimer -= DeltaTime;
		if (BoostTimer <= 0.0f)
		{
			bIsBoosting = false;
			BoostTimer = 0.0f;
		}
	}
}

void UPowderMovementComponent::ApplyMovement(float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	// Forward movement along slope
	float EffectiveSpeed = CurrentSpeed;
	if (bIsBoosting)
	{
		EffectiveSpeed += BoostBurstSpeed;
	}

	// Movement direction is based on persistent heading (DesiredYaw)
	FVector MoveDirection = FRotator(0.0f, DesiredYaw, 0.0f).Vector();
	// Project onto slope plane to stay on terrain
	MoveDirection = FVector::VectorPlaneProject(MoveDirection, SlopeNormal).GetSafeNormal();
	FVector TotalMovement = MoveDirection * EffectiveSpeed * DeltaTime;

	// Lateral drift/slide during carves — scales with forward speed (no drift when stationary)
	if (CarveLateralSpeed > 0.0f && FMath::Abs(CurrentCarveAngle) > 1.0f && EffectiveSpeed > 1.0f)
	{
		FVector RightVector = FVector::CrossProduct(SlopeNormal, MoveDirection).GetSafeNormal();
		float CarveNorm = CurrentCarveAngle / MaxCarveAngle;
		float LateralFactor = FMath::Clamp(EffectiveSpeed / FMath::Max(1.0f, MaxSpeed), 0.0f, 1.0f);
		TotalMovement += RightVector * CarveNorm * CarveLateralSpeed * LateralFactor * DeltaTime;
	}

	Velocity = TotalMovement / DeltaTime;

	// Visual yaw smoothing: movement uses DesiredYaw, visual rotation can lag behind
	if (YawSmoothing > 0.0f)
	{
		VisualYaw = FMath::FInterpTo(VisualYaw, DesiredYaw, DeltaTime, 1.0f / YawSmoothing);
	}
	else
	{
		VisualYaw = DesiredYaw;
	}

	// --- Compute lean and pitch in movement space, then compose with mesh offset ---

	// Slope cross-tilt: lean into the hill when traversing across a slope
	FVector CharForward = FRotator(0.0f, DesiredYaw, 0.0f).Vector();
	FVector CharRight = FVector::CrossProduct(FVector::UpVector, CharForward).GetSafeNormal();
	float SlopeLateralComponent = FVector::DotProduct(SlopeNormal, CharRight);
	float SlopeRoll = -FMath::RadiansToDegrees(FMath::Asin(FMath::Clamp(SlopeLateralComponent, -1.0f, 1.0f)));

	// Carve lean: additional roll into the turn, scaled by edge depth and speed
	float CarveNorm = CurrentCarveAngle / FMath::Max(1.0f, MaxCarveAngle);
	float SpeedLeanScale = FMath::Clamp(CurrentSpeed / FMath::Max(1.0f, MaxSpeed), 0.2f, 1.0f);
	float CarveLean = -CarveNorm * CarveLeanMaxAngle * SpeedLeanScale;

	float TargetLean = SlopeRoll + CarveLean;
	SmoothedCarveLean = FMath::FInterpTo(SmoothedCarveLean, TargetLean, DeltaTime, CarveLeanInterpSpeed);

	// Slope-aligned pitch: project movement direction onto slope plane
	FVector MoveOnSlope = FVector::VectorPlaneProject(CharForward, SlopeNormal).GetSafeNormal();
	float SlopePitch = FMath::RadiansToDegrees(FMath::Asin(MoveOnSlope.Z));
	SmoothedSlopePitch = FMath::FInterpTo(SmoothedSlopePitch, SlopePitch, DeltaTime, SlopePitchInterpSpeed);

	// Compose rotation: heading tilt (pitch/roll in movement frame) then mesh offset
	// This ensures lean/pitch axes are correct regardless of VisualYawOffset
	FQuat HeadingQuat = FRotator(0.0f, VisualYaw, 0.0f).Quaternion();
	FQuat TiltQuat = FRotator(SmoothedSlopePitch, 0.0f, SmoothedCarveLean).Quaternion();
	FQuat MeshOffsetQuat = FRotator(0.0f, VisualYawOffset, 0.0f).Quaternion();
	FRotator DesiredRotation = (HeadingQuat * TiltQuat * MeshOffsetQuat).Rotator();

	FHitResult Hit;
	UpdatedComponent->MoveComponent(TotalMovement, DesiredRotation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComponent = Hit.GetComponent();
		const bool bIsJump = Cast<APowderJump>(HitActor) != nullptr;
		const bool bComponentObstacle = HitComponent && HitComponent->ComponentHasTag(FName(TEXT("PowderObstacle")));
		const bool bActorObstacle = HitActor && HitActor->ActorHasTag(FName(TEXT("PowderObstacle")));
		const bool bHasObstacleTag = bComponentObstacle || bActorObstacle;
		const bool bIsObstacle = (!bIsJump && bHasObstacleTag);

		if (bIsObstacle)
		{
			// Obstacle hit = full stop and wipeout
			CurrentSpeed = 0.0f;
			WipeoutRecoveryTimer = 0.3f;
			OnWipeout.Broadcast();

			// Push outward to prevent stuck inside geometry
			FVector PushOut = Hit.ImpactNormal * 50.0f;
			UpdatedComponent->MoveComponent(PushOut, DesiredRotation, true);
		}
		else
		{
			// Non-obstacle hit (terrain edge, ramp, or untagged world geo) — slide along surface
			FVector Remaining = TotalMovement * (1.0f - Hit.Time);
			FVector SlideDir = FVector::VectorPlaneProject(Remaining, Hit.ImpactNormal);
			UpdatedComponent->MoveComponent(SlideDir, DesiredRotation, true);
		}
	}

}

void UPowderMovementComponent::TriggerWipeout()
{
	CurrentSpeed *= 0.5f;
	WipeoutRecoveryTimer = 0.3f;
	OnWipeout.Broadcast();
}

void UPowderMovementComponent::ResetMovementState()
{
	CurrentSpeed = 0.0f;
	BoostMeter = 0.0f;
	CurrentCarveAngle = 0.0f;
	CarveInput = 0.0f;
	bIsAirborne = false;
	AirborneVelocity = FVector::ZeroVector;
	AirborneTimer = 0.0f;
	WipeoutRecoveryTimer = 0.0f;
	bIsBoosting = false;
	BoostTimer = 0.0f;
	DesiredYaw = 0.0f;
	bHeadingInitialized = false;
	SmoothedCarveBleed = 0.0f;
	SmoothedCarveInput = 0.0f;
	VisualYaw = 0.0f;
	LastTurnRateDegPerSec = 0.0f;
	GroundNormalStability = 1.0f;
	OllieCooldownTimer = 0.0f;
	EdgeDepth = 0.0f;
	TurnCommitTimer = 0.0f;
	CommittedCarveAngle = 0.0f;
	LandingPenaltyTimer = 0.0f;
	LandingPenaltyStrength = 0.0f;
	LandingBlendTimer = 0.0f;
	bIsLandingBlend = false;
	EdgeTransitionTimer = 0.0f;
	bInEdgeTransition = false;
	LastCarveSign = 0;
	CarvePressure = 0.0f;
	SmoothedCarveLean = 0.0f;
	SmoothedSlopePitch = 0.0f;
	GroundVerticalVelocity = 0.0f;
	Velocity = FVector::ZeroVector;
	SlopeForward = FVector::ForwardVector;
	SlopeNormal = FVector::UpVector;
}

void UPowderMovementComponent::InitializeHeading(const FVector& CourseDirection)
{
	FVector Dir = FVector(CourseDirection.X, CourseDirection.Y, 0.0f).GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		return;
	}
	SlopeForward = Dir;
	DesiredYaw = Dir.Rotation().Yaw;
	VisualYaw = DesiredYaw;
	bHeadingInitialized = true;

	if (!UpdatedComponent)
	{
		return;
	}

	// Apply rotation immediately so the mesh faces correctly even while frozen
	FRotator Rot = UpdatedComponent->GetComponentRotation();
	Rot.Yaw = VisualYaw + VisualYawOffset;
	UpdatedComponent->SetWorldRotation(Rot);

	// Snap to terrain so the character doesn't float while frozen at spawn
	FVector Loc = UpdatedComponent->GetComponentLocation();
	FVector TraceStart = Loc + FVector(0.0f, 0.0f, 400.0f);
	FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 400.0f + TerrainTraceDistance);
	FHitResult Hit;
	if (TraceForTaggedTerrain(TraceStart, TraceEnd, Hit) && Hit.ImpactNormal.Z > MinGroundNormalZ)
	{
		float DesiredZ = Hit.ImpactPoint.Z + GetOwnerCapsuleHalfHeight() + TerrainContactOffset;
		UpdatedComponent->SetWorldLocation(FVector(Loc.X, Loc.Y, DesiredZ));
	}
}

void UPowderMovementComponent::SetFrozen(bool bFreeze)
{
	bIsFrozen = bFreeze;
	if (bIsFrozen)
	{
		Velocity = FVector::ZeroVector;
		CurrentSpeed = 0.0f;
	}
}

void UPowderMovementComponent::Ollie()
{
	if (bIsAirborne || OllieCooldownTimer > 0.0f)
	{
		return;
	}

	LaunchIntoAir(FVector(0.0f, 0.0f, OllieForce));
	OllieCooldownTimer = OllieCooldown;
}

void UPowderMovementComponent::LaunchIntoAir(FVector AdditionalVelocity)
{
	if (bIsAirborne)
	{
		return;
	}

	bIsAirborne = true;
	AirborneTimer = 0.0f;

	// Combine current ground velocity with launch impulse
	AirborneVelocity = Velocity + AdditionalVelocity;

	OnLaunched.Broadcast();
}

void UPowderMovementComponent::AddAirborneImpulse(FVector Impulse)
{
	if (!bIsAirborne)
	{
		return;
	}
	AirborneVelocity += Impulse;
}

void UPowderMovementComponent::UpdateAirborne(float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	AirborneTimer += DeltaTime;

	// Apply air drag (reduces all axes, damps horizontal drift)
	AirborneVelocity -= AirborneVelocity * AirDragCoefficient * DeltaTime;

	// Apply gravity after drag
	AirborneVelocity.Z -= GravityAcceleration * DeltaTime;

	// Clamp to terminal velocity
	AirborneVelocity.Z = FMath::Max(AirborneVelocity.Z, -AirTerminalVelocity);

	FVector Movement = AirborneVelocity * DeltaTime;
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	FHitResult Hit;
	UpdatedComponent->MoveComponent(Movement, CurrentRotation, true, &Hit);

	bool bLanded = false;

	// Check for landing: blocking hit with upward-facing surface (obstacle or non-slope geometry)
	if (Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z > MinGroundNormalZ)
	{
		bLanded = true;
	}

	// Trace-based landing fallback: slopes ignore Pawn sweeps to prevent seam collision,
	// so we need a line trace to detect when we've descended onto the slope surface
	if (!bLanded && AirborneVelocity.Z < 0.0f)
	{
		FVector CurrentPos = UpdatedComponent->GetComponentLocation();
		FVector TraceStart = CurrentPos + FVector(0.0f, 0.0f, 200.0f);
		FVector TraceEnd = CurrentPos - FVector(0.0f, 0.0f, 200.0f);

		FHitResult TraceHit;
		if (TraceForTaggedTerrain(TraceStart, TraceEnd, TraceHit)
			&& TraceHit.ImpactNormal.Z > MinGroundNormalZ)
		{
			// Ground is within snap distance — land
			float GroundZ = TraceHit.ImpactPoint.Z + GetOwnerCapsuleHalfHeight() + TerrainContactOffset;
			if (CurrentPos.Z <= GroundZ + 20.0f)
			{
				// Snap to ground
				FVector SnapDelta(0.0f, 0.0f, GroundZ - CurrentPos.Z);
				UpdatedComponent->MoveComponent(SnapDelta, CurrentRotation, true);
				bLanded = true;
			}
		}
	}

	if (bLanded)
	{
		bIsAirborne = false;

		// Start landing blend
		bIsLandingBlend = true;
		LandingBlendTimer = LandingBlendDuration;

		// --- Landing quality assessment (Feature 5) ---
		// Perfect landing: velocity parallel to slope (dot with normal ~ 0)
		// Bad landing: velocity into ground (dot with normal ~ -1)
		FVector VelocityDir = AirborneVelocity.GetSafeNormal();
		float LandingDot = FVector::DotProduct(VelocityDir, SlopeNormal);
		// Map: dot 0 (parallel) → quality 1.0, dot -1 (into ground) → quality 0.0
		float LandingQuality = FMath::Clamp(1.0f + LandingDot, 0.0f, 1.0f);

		// Apply threshold: above threshold = perfect (no penalty)
		if (LandingQuality >= LandingQualityThreshold)
		{
			LandingQuality = 1.0f;
		}
		else
		{
			LandingQuality = FMath::GetMappedRangeValueClamped(
				FVector2D(0.0f, LandingQualityThreshold),
				FVector2D(0.0f, 1.0f),
				LandingQuality);
		}

		// Restore speed from horizontal component with landing penalty
		FVector HorizontalVel(AirborneVelocity.X, AirborneVelocity.Y, 0.0f);
		float HorizontalSpeed = HorizontalVel.Size();
		float SpeedPenalty = (1.0f - LandingQuality) * LandingSpeedPenaltyMax;
		CurrentSpeed = FMath::Clamp(HorizontalSpeed * (1.0f - SpeedPenalty), 0.0f, MaxSpeed);

		// Align heading to airborne velocity direction.
		// Speed projection in UpdateCarving naturally prevents wrong-direction travel.
		if (HorizontalSpeed > 10.0f)
		{
			DesiredYaw = HorizontalVel.Rotation().Yaw;
			VisualYaw = DesiredYaw;
		}

		// Set landing control penalty for bad landings
		if (LandingQuality < 1.0f)
		{
			LandingPenaltyStrength = 1.0f - LandingQuality;
			LandingPenaltyTimer = LandingControlPenaltyDuration * LandingPenaltyStrength;
		}

		AirborneVelocity = FVector::ZeroVector;

		float AirTime = AirborneTimer;
		AirborneTimer = 0.0f;

		OnLanded.Broadcast(AirTime, LandingQuality);
	}
}

bool UPowderMovementComponent::TraceForTaggedTerrain(const FVector& Start, const FVector& End, FHitResult& OutTerrainHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
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
		if (bIsTerrain)
		{
			OutTerrainHit = Hit;
			return true;
		}
	}

	return false;
}

IPowderSurfaceQueryProvider* UPowderMovementComponent::ResolveSurfaceQueryProvider(float DeltaTime)
{
	if (UObject* CachedObject = CachedSurfaceProviderObject.Get())
	{
		return Cast<IPowderSurfaceQueryProvider>(CachedObject);
	}

	SurfaceProviderRetryTimer = FMath::Max(0.0f, SurfaceProviderRetryTimer - DeltaTime);
	if (SurfaceProviderRetryTimer > 0.0f)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		SurfaceProviderRetryTimer = 1.0f;
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Candidate = *It;
		if (!Candidate || !Candidate->GetClass()->ImplementsInterface(UPowderSurfaceQueryProvider::StaticClass()))
		{
			continue;
		}

		CachedSurfaceProviderObject = Candidate;
		return Cast<IPowderSurfaceQueryProvider>(Candidate);
	}

	SurfaceProviderRetryTimer = 1.0f;
	return nullptr;
}

float UPowderMovementComponent::GetOwnerCapsuleHalfHeight() const
{
	// Derive the distance from the component pivot to the bottom of its bounding box.
	// Center pivot: Origin.Z=0, Extent.Z=half → returns half (correct).
	// Bottom pivot: Origin.Z=half, Extent.Z=half → returns 0 (correct).
	if (UpdatedComponent)
	{
		FBoxSphereBounds Bounds = UpdatedComponent->CalcBounds(FTransform::Identity);
		return Bounds.BoxExtent.Z - Bounds.Origin.Z;
	}

	return 90.0f;
}

void UPowderMovementComponent::ApplyTuningProfile(const FMovementTuning& Tuning, float BlendTime)
{
	// Snapshot current values as blend start
	TuningBlendStart.GravityAcceleration = GravityAcceleration;
	TuningBlendStart.SlopeAngle = SlopeAngle;
	TuningBlendStart.GravityAlongSlopeScale = GravityAlongSlopeScale;
	TuningBlendStart.MaxAccelerationSlopeAngle = MaxAccelerationSlopeAngle;
	TuningBlendStart.MaxSpeed = MaxSpeed;
	TuningBlendStart.BaseFriction = BaseFriction;
	TuningBlendStart.CarveSpeedBleed = CarveSpeedBleed;
	TuningBlendStart.CarveBleedSmoothing = CarveBleedSmoothing;
	TuningBlendStart.CarveRate = CarveRate;
	TuningBlendStart.CarveReturnRate = CarveReturnRate;
	TuningBlendStart.MaxCarveAngle = MaxCarveAngle;
	TuningBlendStart.YawRate = YawRate;
	TuningBlendStart.CarveLateralSpeed = CarveLateralSpeed;
	TuningBlendStart.BoostFillRate = BoostFillRate;
	TuningBlendStart.BoostBurstSpeed = BoostBurstSpeed;
	TuningBlendStart.BoostDuration = BoostDuration;
	TuningBlendStart.OllieForce = OllieForce;
	TuningBlendStart.OllieCooldown = OllieCooldown;
	TuningBlendStart.CarveInputSmoothing = CarveInputSmoothing;
	TuningBlendStart.CarveRampTime = CarveRampTime;
	TuningBlendStart.CarveRampMinIntensity = CarveRampMinIntensity;
	TuningBlendStart.CarveRampEaseExponent = CarveRampEaseExponent;
	TuningBlendStart.SpeedTurnLimitFactor = SpeedTurnLimitFactor;
	TuningBlendStart.MinTurnAngleAtMaxSpeed = MinTurnAngleAtMaxSpeed;
	TuningBlendStart.YawSmoothing = YawSmoothing;
	TuningBlendStart.MinGroundNormalZ = MinGroundNormalZ;
	TuningBlendStart.GroundNormalFilterSpeed = GroundNormalFilterSpeed;
	TuningBlendStart.DownhillAlignRate = DownhillAlignRate;
	TuningBlendStart.TurnRateLimitDegPerSec = TurnRateLimitDegPerSec;
	TuningBlendStart.CarveBleedExponent = CarveBleedExponent;
	TuningBlendStart.SpeedTurnRateMin = SpeedTurnRateMin;
	TuningBlendStart.SpeedTurnRateExponent = SpeedTurnRateExponent;
	TuningBlendStart.EdgeEngageRate = EdgeEngageRate;
	TuningBlendStart.EdgeDisengageRate = EdgeDisengageRate;
	TuningBlendStart.EdgeMinDepth = EdgeMinDepth;
	TuningBlendStart.HeadingTraverseFactor = HeadingTraverseFactor;
	TuningBlendStart.HeadingUphillFactor = HeadingUphillFactor;
	TuningBlendStart.HeadingFrictionScale = HeadingFrictionScale;
	TuningBlendStart.TurnCommitTime = TurnCommitTime;
	TuningBlendStart.TurnCommitDecay = TurnCommitDecay;
	TuningBlendStart.LandingSpeedPenaltyMax = LandingSpeedPenaltyMax;
	TuningBlendStart.LandingControlPenaltyDuration = LandingControlPenaltyDuration;
	TuningBlendStart.LandingControlPenaltyFactor = LandingControlPenaltyFactor;
	TuningBlendStart.LandingQualityThreshold = LandingQualityThreshold;
	TuningBlendStart.TerrainSnapThreshold = TerrainSnapThreshold;
	TuningBlendStart.LandingBlendDuration = LandingBlendDuration;
	TuningBlendStart.AirDragCoefficient = AirDragCoefficient;
	TuningBlendStart.AirTerminalVelocity = AirTerminalVelocity;
	TuningBlendStart.EdgeTransitionTime = EdgeTransitionTime;
	TuningBlendStart.EdgeTransitionGrip = EdgeTransitionGrip;
	TuningBlendStart.CarvePressureBuildRate = CarvePressureBuildRate;
	TuningBlendStart.CarvePressureDecayRate = CarvePressureDecayRate;
	TuningBlendStart.CarvePressureTurnBonus = CarvePressureTurnBonus;
	TuningBlendStart.CarvePressureBleedBonus = CarvePressureBleedBonus;

	TuningBlendTarget = Tuning;
	TuningBlendDuration = FMath::Max(BlendTime, KINDA_SMALL_NUMBER);
	TuningBlendAlpha = 0.0f;
	bIsBlendingTuning = true;
}

void UPowderMovementComponent::TickTuningBlend(float DeltaTime)
{
	if (!bIsBlendingTuning)
	{
		return;
	}

	TuningBlendAlpha = FMath::Clamp(TuningBlendAlpha + DeltaTime / TuningBlendDuration, 0.0f, 1.0f);
	float Alpha = TuningBlendAlpha;

	GravityAcceleration = FMath::Lerp(TuningBlendStart.GravityAcceleration, TuningBlendTarget.GravityAcceleration, Alpha);
	SlopeAngle = FMath::Lerp(TuningBlendStart.SlopeAngle, TuningBlendTarget.SlopeAngle, Alpha);
	GravityAlongSlopeScale = FMath::Lerp(TuningBlendStart.GravityAlongSlopeScale, TuningBlendTarget.GravityAlongSlopeScale, Alpha);
	MaxAccelerationSlopeAngle = FMath::Lerp(TuningBlendStart.MaxAccelerationSlopeAngle, TuningBlendTarget.MaxAccelerationSlopeAngle, Alpha);
	MaxSpeed = FMath::Lerp(TuningBlendStart.MaxSpeed, TuningBlendTarget.MaxSpeed, Alpha);
	BaseFriction = FMath::Lerp(TuningBlendStart.BaseFriction, TuningBlendTarget.BaseFriction, Alpha);
	CarveSpeedBleed = FMath::Lerp(TuningBlendStart.CarveSpeedBleed, TuningBlendTarget.CarveSpeedBleed, Alpha);
	CarveBleedSmoothing = FMath::Lerp(TuningBlendStart.CarveBleedSmoothing, TuningBlendTarget.CarveBleedSmoothing, Alpha);
	CarveRate = FMath::Lerp(TuningBlendStart.CarveRate, TuningBlendTarget.CarveRate, Alpha);
	CarveReturnRate = FMath::Lerp(TuningBlendStart.CarveReturnRate, TuningBlendTarget.CarveReturnRate, Alpha);
	MaxCarveAngle = FMath::Lerp(TuningBlendStart.MaxCarveAngle, TuningBlendTarget.MaxCarveAngle, Alpha);
	YawRate = FMath::Lerp(TuningBlendStart.YawRate, TuningBlendTarget.YawRate, Alpha);
	CarveLateralSpeed = FMath::Lerp(TuningBlendStart.CarveLateralSpeed, TuningBlendTarget.CarveLateralSpeed, Alpha);
	BoostFillRate = FMath::Lerp(TuningBlendStart.BoostFillRate, TuningBlendTarget.BoostFillRate, Alpha);
	BoostBurstSpeed = FMath::Lerp(TuningBlendStart.BoostBurstSpeed, TuningBlendTarget.BoostBurstSpeed, Alpha);
	BoostDuration = FMath::Lerp(TuningBlendStart.BoostDuration, TuningBlendTarget.BoostDuration, Alpha);
	OllieForce = FMath::Lerp(TuningBlendStart.OllieForce, TuningBlendTarget.OllieForce, Alpha);
	OllieCooldown = FMath::Lerp(TuningBlendStart.OllieCooldown, TuningBlendTarget.OllieCooldown, Alpha);
	CarveInputSmoothing = FMath::Lerp(TuningBlendStart.CarveInputSmoothing, TuningBlendTarget.CarveInputSmoothing, Alpha);
	CarveRampTime = FMath::Lerp(TuningBlendStart.CarveRampTime, TuningBlendTarget.CarveRampTime, Alpha);
	CarveRampMinIntensity = FMath::Lerp(TuningBlendStart.CarveRampMinIntensity, TuningBlendTarget.CarveRampMinIntensity, Alpha);
	CarveRampEaseExponent = FMath::Lerp(TuningBlendStart.CarveRampEaseExponent, TuningBlendTarget.CarveRampEaseExponent, Alpha);
	SpeedTurnLimitFactor = FMath::Lerp(TuningBlendStart.SpeedTurnLimitFactor, TuningBlendTarget.SpeedTurnLimitFactor, Alpha);
	MinTurnAngleAtMaxSpeed = FMath::Lerp(TuningBlendStart.MinTurnAngleAtMaxSpeed, TuningBlendTarget.MinTurnAngleAtMaxSpeed, Alpha);
	YawSmoothing = FMath::Lerp(TuningBlendStart.YawSmoothing, TuningBlendTarget.YawSmoothing, Alpha);
	MinGroundNormalZ = FMath::Lerp(TuningBlendStart.MinGroundNormalZ, TuningBlendTarget.MinGroundNormalZ, Alpha);
	GroundNormalFilterSpeed = FMath::Lerp(TuningBlendStart.GroundNormalFilterSpeed, TuningBlendTarget.GroundNormalFilterSpeed, Alpha);
	DownhillAlignRate = FMath::Lerp(TuningBlendStart.DownhillAlignRate, TuningBlendTarget.DownhillAlignRate, Alpha);
	TurnRateLimitDegPerSec = FMath::Lerp(TuningBlendStart.TurnRateLimitDegPerSec, TuningBlendTarget.TurnRateLimitDegPerSec, Alpha);
	CarveBleedExponent = FMath::Lerp(TuningBlendStart.CarveBleedExponent, TuningBlendTarget.CarveBleedExponent, Alpha);
	SpeedTurnRateMin = FMath::Lerp(TuningBlendStart.SpeedTurnRateMin, TuningBlendTarget.SpeedTurnRateMin, Alpha);
	SpeedTurnRateExponent = FMath::Lerp(TuningBlendStart.SpeedTurnRateExponent, TuningBlendTarget.SpeedTurnRateExponent, Alpha);
	EdgeEngageRate = FMath::Lerp(TuningBlendStart.EdgeEngageRate, TuningBlendTarget.EdgeEngageRate, Alpha);
	EdgeDisengageRate = FMath::Lerp(TuningBlendStart.EdgeDisengageRate, TuningBlendTarget.EdgeDisengageRate, Alpha);
	EdgeMinDepth = FMath::Lerp(TuningBlendStart.EdgeMinDepth, TuningBlendTarget.EdgeMinDepth, Alpha);
	HeadingTraverseFactor = FMath::Lerp(TuningBlendStart.HeadingTraverseFactor, TuningBlendTarget.HeadingTraverseFactor, Alpha);
	HeadingUphillFactor = FMath::Lerp(TuningBlendStart.HeadingUphillFactor, TuningBlendTarget.HeadingUphillFactor, Alpha);
	HeadingFrictionScale = FMath::Lerp(TuningBlendStart.HeadingFrictionScale, TuningBlendTarget.HeadingFrictionScale, Alpha);
	TurnCommitTime = FMath::Lerp(TuningBlendStart.TurnCommitTime, TuningBlendTarget.TurnCommitTime, Alpha);
	TurnCommitDecay = FMath::Lerp(TuningBlendStart.TurnCommitDecay, TuningBlendTarget.TurnCommitDecay, Alpha);
	LandingSpeedPenaltyMax = FMath::Lerp(TuningBlendStart.LandingSpeedPenaltyMax, TuningBlendTarget.LandingSpeedPenaltyMax, Alpha);
	LandingControlPenaltyDuration = FMath::Lerp(TuningBlendStart.LandingControlPenaltyDuration, TuningBlendTarget.LandingControlPenaltyDuration, Alpha);
	LandingControlPenaltyFactor = FMath::Lerp(TuningBlendStart.LandingControlPenaltyFactor, TuningBlendTarget.LandingControlPenaltyFactor, Alpha);
	LandingQualityThreshold = FMath::Lerp(TuningBlendStart.LandingQualityThreshold, TuningBlendTarget.LandingQualityThreshold, Alpha);
	TerrainSnapThreshold = FMath::Lerp(TuningBlendStart.TerrainSnapThreshold, TuningBlendTarget.TerrainSnapThreshold, Alpha);
	LandingBlendDuration = FMath::Lerp(TuningBlendStart.LandingBlendDuration, TuningBlendTarget.LandingBlendDuration, Alpha);
	AirDragCoefficient = FMath::Lerp(TuningBlendStart.AirDragCoefficient, TuningBlendTarget.AirDragCoefficient, Alpha);
	AirTerminalVelocity = FMath::Lerp(TuningBlendStart.AirTerminalVelocity, TuningBlendTarget.AirTerminalVelocity, Alpha);
	EdgeTransitionTime = FMath::Lerp(TuningBlendStart.EdgeTransitionTime, TuningBlendTarget.EdgeTransitionTime, Alpha);
	EdgeTransitionGrip = FMath::Lerp(TuningBlendStart.EdgeTransitionGrip, TuningBlendTarget.EdgeTransitionGrip, Alpha);
	CarvePressureBuildRate = FMath::Lerp(TuningBlendStart.CarvePressureBuildRate, TuningBlendTarget.CarvePressureBuildRate, Alpha);
	CarvePressureDecayRate = FMath::Lerp(TuningBlendStart.CarvePressureDecayRate, TuningBlendTarget.CarvePressureDecayRate, Alpha);
	CarvePressureTurnBonus = FMath::Lerp(TuningBlendStart.CarvePressureTurnBonus, TuningBlendTarget.CarvePressureTurnBonus, Alpha);
	CarvePressureBleedBonus = FMath::Lerp(TuningBlendStart.CarvePressureBleedBonus, TuningBlendTarget.CarvePressureBleedBonus, Alpha);

	if (Alpha >= 1.0f)
	{
		bIsBlendingTuning = false;
	}
}

void UPowderMovementComponent::SetCurrentSurface(const FSurfaceProperties& NewSurface)
{
	if (NewSurface.Type == CurrentSurface.Type && !bIsBlendingSurface)
	{
		return;
	}

	SurfaceBlendStart = CurrentSurface;
	SurfaceBlendTarget = NewSurface;
	SurfaceBlendDuration = FMath::Max(NewSurface.BlendTime, KINDA_SMALL_NUMBER);
	SurfaceBlendAlpha = 0.0f;
	bIsBlendingSurface = true;
}

void UPowderMovementComponent::TickSurfaceBlend(float DeltaTime)
{
	if (!bIsBlendingSurface)
	{
		return;
	}

	SurfaceBlendAlpha = FMath::Clamp(SurfaceBlendAlpha + DeltaTime / SurfaceBlendDuration, 0.0f, 1.0f);
	float Alpha = SurfaceBlendAlpha;

	CurrentSurface.Friction = FMath::Lerp(SurfaceBlendStart.Friction, SurfaceBlendTarget.Friction, Alpha);
	CurrentSurface.CarveGrip = FMath::Lerp(SurfaceBlendStart.CarveGrip, SurfaceBlendTarget.CarveGrip, Alpha);
	CurrentSurface.SpeedMultiplier = FMath::Lerp(SurfaceBlendStart.SpeedMultiplier, SurfaceBlendTarget.SpeedMultiplier, Alpha);
	CurrentSurface.SnowSprayAmount = FMath::Lerp(SurfaceBlendStart.SnowSprayAmount, SurfaceBlendTarget.SnowSprayAmount, Alpha);
	CurrentSurface.SprayColor = FMath::Lerp(SurfaceBlendStart.SprayColor, SurfaceBlendTarget.SprayColor, Alpha);
	CurrentSurface.SurfaceColor = FMath::Lerp(SurfaceBlendStart.SurfaceColor, SurfaceBlendTarget.SurfaceColor, Alpha);

	if (Alpha >= 1.0f)
	{
		CurrentSurface.Type = SurfaceBlendTarget.Type;
		bIsBlendingSurface = false;
	}
}

