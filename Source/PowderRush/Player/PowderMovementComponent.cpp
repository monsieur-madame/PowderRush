#include "Player/PowderMovementComponent.h"
#include "Components/PrimitiveComponent.h"

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
	// Smoothly interpolate carve angle toward target
	float TargetAngle = CarveInput * MaxCarveAngle;
	float InterpSpeed = CarveRate;

	// Return to center faster than carving in (snappy response)
	if (FMath::Abs(TargetAngle) < FMath::Abs(CurrentCarveAngle))
	{
		InterpSpeed *= 2.0f;
	}

	CurrentCarveAngle = FMath::FInterpTo(CurrentCarveAngle, TargetAngle, DeltaTime, InterpSpeed);

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

	// Carve speed bleed - deeper carves lose more speed
	float CarveBleed = 0.0f;
	if (IsCarving())
	{
		float CarveDepth = FMath::Abs(CurrentCarveAngle) / MaxCarveAngle;
		CarveBleed = CarveSpeedBleed * CarveDepth;
	}

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

	FVector ForwardMovement = SlopeForward * EffectiveSpeed * DeltaTime;

	// Lateral movement from carving
	FVector Right = FVector::CrossProduct(SlopeNormal, SlopeForward).GetSafeNormal();
	float LateralAmount = FMath::Sin(FMath::DegreesToRadians(CurrentCarveAngle));
	float LateralMod = EquipmentStats.CarveMultiplier;
	FVector LateralMovement = Right * LateralAmount * CarveLateralSpeed * LateralMod * DeltaTime;

	// Combine and apply
	FVector TotalMovement = ForwardMovement + LateralMovement;
	Velocity = TotalMovement / DeltaTime;

	// Rotate capsule to face movement direction — Yaw only, no Pitch/Roll
	// Visual tilt is handled by the spring arm camera in PowderCharacter::Tick
	FRotator DesiredRotation = UpdatedComponent->GetComponentRotation();
	DesiredRotation.Pitch = 0.0f;
	DesiredRotation.Roll = 0.0f;
	if (!SlopeForward.IsNearlyZero())
	{
		// Stable yaw: rotate SlopeForward by carve angle around Up axis
		FVector CarvedDirection = SlopeForward.RotateAngleAxis(CurrentCarveAngle, FVector::UpVector);
		DesiredYaw = CarvedDirection.Rotation().Yaw;
		DesiredRotation.Yaw = DesiredYaw;
	}

	FHitResult Hit;
	UpdatedComponent->MoveComponent(TotalMovement, DesiredRotation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Slide along surfaces we hit
		FVector SlideMovement = FVector::VectorPlaneProject(TotalMovement, Hit.ImpactNormal);
		FHitResult SlideHit;
		UpdatedComponent->MoveComponent(SlideMovement, DesiredRotation, true, &SlideHit);

		if (SlideHit.IsValidBlockingHit())
		{
			// Both movement attempts blocked — push outward to prevent stuck state
			FVector PushOut = Hit.ImpactNormal * 50.0f;
			UpdatedComponent->MoveComponent(PushOut, DesiredRotation, true);
		}

		// If we hit something head-on at speed, trigger wipeout
		float ImpactDot = FVector::DotProduct(TotalMovement.GetSafeNormal(), Hit.ImpactNormal);
		if (ImpactDot < -0.7f && CurrentSpeed > MaxSpeed * 0.5f)
		{
			CurrentSpeed *= 0.1f;
			WipeoutRecoveryTimer = 0.5f;
			OnWipeout.Broadcast();
		}
	}

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
