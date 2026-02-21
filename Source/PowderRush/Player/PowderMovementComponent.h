#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Core/PowderTypes.h"
#include "PowderMovementComponent.generated.h"

UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UPowderMovementComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

	// --- Input ---
	// CarveInput: -1.0 = full left, 0.0 = straight, 1.0 = full right
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void SetCarveInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void ReleaseCarve();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void ActivateBoost();

	/** Fire-and-forget speed boost from powerup pickup (bypasses BoostMeter). */
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void TriggerSpeedBoost(float BurstSpeed, float Duration);

	// --- State Queries ---
	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetCurrentSpeed() const { return CurrentSpeed; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetSpeedNormalized() const;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetBoostMeter() const { return BoostMeter; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetCarveAngle() const { return CurrentCarveAngle; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	bool IsCarving() const { return FMath::Abs(CarveInput) > 0.1f; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	bool IsBoosting() const { return bIsBoosting; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	bool IsAirborne() const { return bIsAirborne; }

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void LaunchIntoAir(FVector AdditionalVelocity);

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void Ollie();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void TriggerWipeout();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void ResetMovementState();

	/** Set initial heading from a world direction (e.g. course tangent at spawn). */
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void InitializeHeading(const FVector& CourseDirection);

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void SetFrozen(bool bFreeze);

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	bool IsFrozen() const { return bIsFrozen; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetSlopeForwardYaw() const { return SlopeForward.Rotation().Yaw; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetDesiredYaw() const { return DesiredYaw; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetTurnRateDegPerSec() const { return LastTurnRateDegPerSec; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetGroundNormalStability() const { return GroundNormalStability; }

	// --- Tuning Profile ---
	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement|Tuning")
	void ApplyTuningProfile(const FMovementTuning& Tuning, float BlendTime);

	// --- Events ---
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoostActivated);
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Movement")
	FOnBoostActivated OnBoostActivated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWipeout);
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Movement")
	FOnWipeout OnWipeout;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLaunched);
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Movement")
	FOnLaunched OnLaunched;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLanded, float, AirTime, float, LandingQuality);
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Movement")
	FOnLanded OnLanded;

	// --- Tuning Parameters (exposed to Blueprints for iteration) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float GravityAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	/** Fallback-only angle used when we have no valid terrain normal sample. */
	float SlopeAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float GravityAlongSlopeScale = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MaxAccelerationSlopeAngle = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float BaseFriction = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveSpeedBleed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRate = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveReturnRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MaxCarveAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float YawRate = 90.0f;  // Degrees per second at full carve input

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveLateralSpeed = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveBleedSmoothing = 1.5f;  // How fast speed penalty ramps up/down (lower = smoother transitions)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float BoostFillRate = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float BoostBurstSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float BoostDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TerrainTraceDistance = 1000.0f;

	/** Vertical offset applied after terrain trace snap. Negative values reduce visible hovering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TerrainContactOffset = -6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float OllieForce = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float OllieCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveInputSmoothing = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampMinIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampEaseExponent = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SpeedTurnLimitFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MinTurnAngleAtMaxSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float YawSmoothing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SlopeForwardInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MinGroundNormalZ = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float GroundNormalFilterSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float DownhillAlignRate = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TurnRateLimitDegPerSec = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveBleedExponent = 1.8f;

	// --- Ski Feel ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SpeedTurnRateMin = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SpeedTurnRateExponent = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float EdgeEngageRate = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float EdgeDisengageRate = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float EdgeMinDepth = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float HeadingTraverseFactor = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float HeadingUphillFactor = -0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float HeadingFrictionScale = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TurnCommitTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TurnCommitDecay = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float LandingSpeedPenaltyMax = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float LandingControlPenaltyDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float LandingControlPenaltyFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float LandingQualityThreshold = 0.85f;

	// --- Terrain & Air ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float TerrainSnapThreshold = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float LandingBlendDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float AirDragCoefficient = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float AirTerminalVelocity = 1500.0f;

	// --- Edge Feel ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float EdgeTransitionTime = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float EdgeTransitionGrip = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarvePressureBuildRate = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarvePressureDecayRate = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarvePressureTurnBonus = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarvePressureBleedBonus = 0.2f;

	// --- Equipment Stats (applied from equipped gear) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Equipment")
	FEquipmentStats EquipmentStats;

	// --- Current Surface ---
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Movement")
	FSurfaceProperties CurrentSurface;

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void SetCurrentSurface(const FSurfaceProperties& NewSurface);

protected:
	void UpdateTerrainFollowing(float DeltaTime);
	void UpdateCarving(float DeltaTime);
	void UpdateSpeed(float DeltaTime);
	void UpdateBoost(float DeltaTime);
	void ApplyMovement(float DeltaTime);
	void UpdateAirborne(float DeltaTime);
	bool TraceForTaggedTerrain(const FVector& Start, const FVector& End, FHitResult& OutTerrainHit) const;
	class IPowderSurfaceQueryProvider* ResolveSurfaceQueryProvider(float DeltaTime);
	float GetOwnerCapsuleHalfHeight() const;

	float CarveInput = 0.0f;
	float CurrentCarveAngle = 0.0f;
	float CurrentSpeed = 0.0f;
	float BoostMeter = 0.0f;
	bool bIsBoosting = false;
	float BoostTimer = 0.0f;
	FVector SlopeNormal = FVector::UpVector;
	FVector SlopeForward = FVector::ForwardVector;
	float DesiredYaw = 0.0f;
	bool bHeadingInitialized = false;
	bool bOnGround = false;
	float SmoothedCarveBleed = 0.0f;
	float SmoothedCarveInput = 0.0f;
	float VisualYaw = 0.0f;
	float LastTurnRateDegPerSec = 0.0f;
	float GroundNormalStability = 1.0f;

	// Ski feel state
	float EdgeDepth = 0.0f;
	float TurnCommitTimer = 0.0f;
	float CommittedCarveAngle = 0.0f;
	float LandingPenaltyTimer = 0.0f;
	float LandingPenaltyStrength = 0.0f;

	// Landing blend state
	float LandingBlendTimer = 0.0f;
	bool bIsLandingBlend = false;

	// Edge transition state
	float EdgeTransitionTimer = 0.0f;
	bool bInEdgeTransition = false;
	int32 LastCarveSign = 0;

	// Carve pressure state
	float CarvePressure = 0.0f;

	// Wipeout recovery
	float WipeoutRecoveryTimer = 0.0f;

	// Ollie cooldown
	float OllieCooldownTimer = 0.0f;

	// Frozen state (movement disabled during menus/pause/wipeout)
	bool bIsFrozen = false;

	// Airborne state
	bool bIsAirborne = false;
	FVector AirborneVelocity = FVector::ZeroVector;
	float AirborneTimer = 0.0f;

	// Surface blend state
	void TickSurfaceBlend(float DeltaTime);
	bool bIsBlendingSurface = false;
	float SurfaceBlendAlpha = 0.0f;
	float SurfaceBlendDuration = 0.5f;
	FSurfaceProperties SurfaceBlendStart;
	FSurfaceProperties SurfaceBlendTarget;

	// Tuning blend state
	void TickTuningBlend(float DeltaTime);
	bool bIsBlendingTuning = false;
	float TuningBlendAlpha = 0.0f;
	float TuningBlendDuration = 1.0f;
	FMovementTuning TuningBlendTarget;
	FMovementTuning TuningBlendStart;

	TWeakObjectPtr<UObject> CachedSurfaceProviderObject;
	float SurfaceProviderRetryTimer = 0.0f;
};
