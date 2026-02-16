#include "Player/PowderMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

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
}

void UPowderMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (bIsAirborne)
	{
		UpdateAirborne(DeltaTime);
		return;
	}

	TickTuningBlend(DeltaTime);
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
	// Start trace from well above the character to avoid starting inside geometry
	FVector Start = CurrentLocation + FVector::UpVector * 200.0f;
	FVector End = Start - FVector::UpVector * (200.0f + TerrainTraceDistance);

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams)
		&& Hit.ImpactNormal.Z > 0.5f)  // Only follow upward-facing surfaces — reject walls, edges, undersides
	{
		bOnGround = true;
		SlopeNormal = Hit.ImpactNormal;

		// Derive downhill direction from gravity projected onto slope — works for any slope orientation
		FVector Gravity = FVector(0.0f, 0.0f, -1.0f);
		FVector Downhill = FVector::VectorPlaneProject(Gravity, SlopeNormal).GetSafeNormal();
		if (!Downhill.IsNearlyZero())
		{
			SlopeForward = Downhill;
		}

		// Immediate snap to terrain surface (capsule half-height offset)
		float DesiredHeight = Hit.ImpactPoint.Z + 90.0f;
		float HeightDelta = DesiredHeight - CurrentLocation.Z;
		FVector HeightAdjust = FVector(0.0f, 0.0f, HeightDelta);
		UpdatedComponent->MoveComponent(HeightAdjust, UpdatedComponent->GetComponentRotation(), true);

		// Initialize heading from slope on first ground contact
		if (DesiredYaw == 0.0f && !SlopeForward.IsNearlyZero())
		{
			DesiredYaw = SlopeForward.Rotation().Yaw;
		}
	}
	else
	{
		// No valid terrain hit — apply gravity so the character settles onto terrain
		bOnGround = false;
		FVector GravityDrop = FVector(0.0f, 0.0f, -GravityAcceleration * DeltaTime);
		UpdatedComponent->MoveComponent(GravityDrop, UpdatedComponent->GetComponentRotation(), true);
	}
}

void UPowderMovementComponent::UpdateCarving(float DeltaTime)
{
	float TargetAngle = CarveInput * MaxCarveAngle;
	float InterpSpeed = CarveRate;

	// Use separate (slower) return rate when releasing input
	if (FMath::Abs(TargetAngle) < FMath::Abs(CurrentCarveAngle))
	{
		InterpSpeed = CarveReturnRate;
	}

	CurrentCarveAngle = FMath::FInterpTo(CurrentCarveAngle, TargetAngle, DeltaTime, InterpSpeed);

	// Derive DesiredYaw from slope forward + carve angle offset
	DesiredYaw = SlopeForward.Rotation().Yaw + CurrentCarveAngle;

	// Build boost meter while actively carving
	if (IsCarving() && CurrentSpeed > MaxSpeed * 0.2f)
	{
		float CarveIntensity = FMath::Abs(CarveInput);
		BoostMeter = FMath::Clamp(BoostMeter + BoostFillRate * CarveIntensity * DeltaTime, 0.0f, 1.0f);
	}
}

void UPowderMovementComponent::UpdateSpeed(float DeltaTime)
{
	if (WipeoutRecoveryTimer > 0.0f)
	{
		WipeoutRecoveryTimer -= DeltaTime;
		return;
	}

	// Gravity-based acceleration along slope
	float SlopeComponent = FMath::Sin(FMath::DegreesToRadians(SlopeAngle));
	float GravityForce = GravityAcceleration * SlopeComponent;

	// Surface friction
	float SurfaceFriction = BaseFriction * CurrentSurface.Friction;

	// Carve speed bleed - based on heading deviation from downhill, smoothed to prevent sudden speed changes
	float HeadingDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(SlopeForward.Rotation().Yaw, DesiredYaw));
	float CarveDepth = FMath::Clamp(HeadingDelta / 90.0f, 0.0f, 1.0f);
	float RawCarveBleed = CarveSpeedBleed * CarveDepth;
	SmoothedCarveBleed = FMath::FInterpTo(SmoothedCarveBleed, RawCarveBleed, DeltaTime, CarveBleedSmoothing);
	float CarveBleed = SmoothedCarveBleed;

	// Apply equipment modifiers
	float SpeedMod = EquipmentStats.SpeedMultiplier;

	// Calculate net acceleration
	float Acceleration = (GravityForce - (SurfaceFriction + CarveBleed) * CurrentSpeed) * SpeedMod;
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
	Velocity = TotalMovement / DeltaTime;

	// Rotate capsule to face heading — Yaw only
	FRotator DesiredRotation = UpdatedComponent->GetComponentRotation();
	DesiredRotation.Pitch = 0.0f;
	DesiredRotation.Roll = 0.0f;
	DesiredRotation.Yaw = DesiredYaw;

	FHitResult Hit;
	UpdatedComponent->MoveComponent(TotalMovement, DesiredRotation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Any obstacle hit = full stop and wipeout
		CurrentSpeed = 0.0f;
		WipeoutRecoveryTimer = 0.3f;
		OnWipeout.Broadcast();

		// Push outward to prevent stuck inside geometry
		FVector PushOut = Hit.ImpactNormal * 50.0f;
		UpdatedComponent->MoveComponent(PushOut, DesiredRotation, true);
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
	SmoothedCarveBleed = 0.0f;
	Velocity = FVector::ZeroVector;
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

void UPowderMovementComponent::UpdateAirborne(float DeltaTime)
{
	if (!UpdatedComponent)
	{
		return;
	}

	AirborneTimer += DeltaTime;

	// Apply gravity
	AirborneVelocity.Z -= GravityAcceleration * DeltaTime;

	FVector Movement = AirborneVelocity * DeltaTime;
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();

	FHitResult Hit;
	UpdatedComponent->MoveComponent(Movement, CurrentRotation, true, &Hit);

	// Check for landing: blocking hit with upward-facing surface
	if (Hit.IsValidBlockingHit() && Hit.ImpactNormal.Z > 0.5f)
	{
		bIsAirborne = false;

		// Restore speed from horizontal component of airborne velocity
		FVector HorizontalVel(AirborneVelocity.X, AirborneVelocity.Y, 0.0f);
		CurrentSpeed = FMath::Clamp(HorizontalVel.Size(), 0.0f, MaxSpeed);
		AirborneVelocity = FVector::ZeroVector;

		float AirTime = AirborneTimer;
		AirborneTimer = 0.0f;

		OnLanded.Broadcast(AirTime);
	}
}

void UPowderMovementComponent::ApplyTuningProfile(const FMovementTuning& Tuning, float BlendTime)
{
	// Snapshot current values as blend start
	TuningBlendStart.GravityAcceleration = GravityAcceleration;
	TuningBlendStart.SlopeAngle = SlopeAngle;
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

	if (Alpha >= 1.0f)
	{
		bIsBlendingTuning = false;
	}
}
