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

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Movement")
	void SetFrozen(bool bFreeze);

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	bool IsFrozen() const { return bIsFrozen; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetSlopeForwardYaw() const { return SlopeForward.Rotation().Yaw; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Movement")
	float GetDesiredYaw() const { return DesiredYaw; }

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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanded, float, AirTime);
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Movement")
	FOnLanded OnLanded;

	// --- Tuning Parameters (exposed to Blueprints for iteration) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float GravityAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SlopeAngle = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float BaseFriction = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveSpeedBleed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRate = 1.0f;

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
	float TerrainTraceDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float OllieForce = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float OllieCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveInputSmoothing = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampMinIntensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float CarveRampEaseExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float SpeedTurnLimitFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float MinTurnAngleAtMaxSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Tuning")
	float YawSmoothing = 0.0f;

	// --- Equipment Stats (applied from equipped gear) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Movement|Equipment")
	FEquipmentStats EquipmentStats;

	// --- Current Surface ---
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Movement")
	FSurfaceProperties CurrentSurface;

protected:
	void UpdateTerrainFollowing(float DeltaTime);
	void UpdateCarving(float DeltaTime);
	void UpdateSpeed(float DeltaTime);
	void UpdateBoost(float DeltaTime);
	void ApplyMovement(float DeltaTime);
	void UpdateAirborne(float DeltaTime);

	float CarveInput = 0.0f;
	float CurrentCarveAngle = 0.0f;
	float CurrentSpeed = 0.0f;
	float BoostMeter = 0.0f;
	bool bIsBoosting = false;
	float BoostTimer = 0.0f;
	FVector SlopeNormal = FVector::UpVector;
	FVector SlopeForward = FVector::ForwardVector;
	float DesiredYaw = 0.0f;
	bool bOnGround = false;
	float SmoothedCarveBleed = 0.0f;
	float SmoothedCarveInput = 0.0f;
	float VisualYaw = 0.0f;

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

	// Tuning blend state
	void TickTuningBlend(float DeltaTime);
	bool bIsBlendingTuning = false;
	float TuningBlendAlpha = 0.0f;
	float TuningBlendDuration = 1.0f;
	FMovementTuning TuningBlendTarget;
	FMovementTuning TuningBlendStart;
};
