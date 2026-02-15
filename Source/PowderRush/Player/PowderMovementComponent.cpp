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

	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start - FVector::UpVector * TerrainTraceDistance;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, QueryParams))
	{
		SlopeNormal = Hit.ImpactNormal;

		// Project forward direction onto slope plane
		FVector WorldForward = -FVector::ForwardVector; // Downhill is -X in our setup
		SlopeForward = FVector::VectorPlaneProject(WorldForward, SlopeNormal).GetSafeNormal();

		// Snap to terrain surface (keep small offset above ground)
		float DesiredHeight = Hit.ImpactPoint.Z + 90.0f; // Capsule half-height offset
		FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
		float HeightDelta = DesiredHeight - CurrentLocation.Z;
		float SnapSpeed = 10.0f;

		FVector HeightAdjust = FVector(0.0f, 0.0f, HeightDelta * FMath::Min(SnapSpeed * DeltaTime, 1.0f));
		UpdatedComponent->MoveComponent(HeightAdjust, UpdatedComponent->GetComponentRotation(), true);
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

	// Rotate character to face movement direction
	FRotator DesiredRotation = UpdatedComponent->GetComponentRotation();
	if (!TotalMovement.IsNearlyZero())
	{
		FRotator MoveRotation = TotalMovement.Rotation();
		DesiredRotation.Yaw = MoveRotation.Yaw;
		// Add visual roll from carving
		DesiredRotation.Roll = -CurrentCarveAngle * 0.3f;
	}

	FHitResult Hit;
	UpdatedComponent->MoveComponent(TotalMovement, DesiredRotation, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		// Slide along surfaces we hit
		FVector SlideMovement = FVector::VectorPlaneProject(TotalMovement, Hit.ImpactNormal);
		UpdatedComponent->MoveComponent(SlideMovement, DesiredRotation, true);

		// If we hit something head-on at speed, trigger wipeout
		float ImpactDot = FVector::DotProduct(TotalMovement.GetSafeNormal(), Hit.ImpactNormal);
		if (ImpactDot < -0.7f && CurrentSpeed > MaxSpeed * 0.5f)
		{
			CurrentSpeed = 0.0f;
			OnWipeout.Broadcast();
		}
	}
}
