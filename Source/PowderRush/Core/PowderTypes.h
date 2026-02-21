// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PowderTypes.generated.h"


UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
	Powder,
	Ice,
	Moguls,
	Groomed
};

UENUM(BlueprintType)
enum class ERiderType : uint8
{
	Skier,
	Snowboarder
};

// --- Powerup Types ---

UENUM(BlueprintType)
enum class EPowerupType : uint8
{
	SpeedBoost,
	ScoreMultiplier
};

// --- Trick System Types ---

UENUM(BlueprintType)
enum class EPowderTrickType : uint8
{
	None,
	Backflip,
	Frontflip,
	HeliSpinLeft,
	HeliSpinRight,
	SpreadEagle
};

UENUM(BlueprintType)
enum class EPowderTrickState : uint8
{
	Idle,
	Executing,
	Completed,
	Failed
};

UENUM(BlueprintType)
enum class EPowderGestureDirection : uint8
{
	None,
	Up,
	Down,
	Left,
	Right,
	HoldBoth,
	// Phase 3f stubs: 8-direction gesture extensions
	UpLeft,
	UpRight,
	DownLeft,
	DownRight,
	DoubleTap,
	CircleCW,
	CircleCCW
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FPowderTrickDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	EPowderTrickType TrickType = EPowderTrickType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	EPowderGestureDirection RequiredGesture = EPowderGestureDirection::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	int32 BasePoints = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	float Duration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	FRotator SpinRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	FName DisplayName;
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FPowderTrickResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Tricks")
	TArray<EPowderTrickType> TricksPerformed;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Tricks")
	int32 TotalPoints = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Tricks")
	bool bAllCompleted = true;
};

// --- Tuning Profile Structs ---

USTRUCT(BlueprintType)
struct POWDERRUSH_API FMovementTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GravityAcceleration = 980.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SlopeAngle = 15.0f;

	/** Scales gravity contribution along slope for gameplay feel (not physical realism). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GravityAlongSlopeScale = 0.45f;

	/** Caps effective slope angle used for acceleration so very steep terrain does not over-accelerate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxAccelerationSlopeAngle = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseFriction = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveSpeedBleed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveBleedSmoothing = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveReturnRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxCarveAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float YawRate = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveLateralSpeed = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostFillRate = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostBurstSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float OllieForce = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float OllieCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveInputSmoothing = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampMinIntensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampEaseExponent = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnLimitFactor = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MinTurnAngleAtMaxSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float YawSmoothing = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MinGroundNormalZ = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GroundNormalFilterSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float DownhillAlignRate = 55.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnRateLimitDegPerSec = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveBleedExponent = 1.8f;

	// --- Ski Feel ---

	/** Fraction of TurnRateLimitDegPerSec at MaxSpeed. Lower = wider arcs at speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnRateMin = 0.35f;

	/** Curve shape for speed-vs-turn-rate mapping (>1 = drops faster at high speed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnRateExponent = 1.2f;

	/** Rate at which edge authority approaches 1.0 (~0.33s at 3.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeEngageRate = 3.0f;

	/** Rate at which edge authority falls back to 0.0 (~0.25s at 4.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeDisengageRate = 4.0f;

	/** Immediate minimum edge authority so taps aren't dead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeMinDepth = 0.3f;

	/** Gravity factor when heading perpendicular to fall line (0 = no gravity, 1 = full). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingTraverseFactor = 0.15f;

	/** Gravity factor when heading uphill (negative = braking force). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingUphillFactor = -0.3f;

	/** Friction reduction when traversing (compensates for less gravity). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingFrictionScale = 0.5f;

	/** Seconds the carve arc continues after input release. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnCommitTime = 0.2f;

	/** How much the committed arc decays during the commit window (0 = hold exact, 1 = linear fade). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnCommitDecay = 0.3f;

	/** Max speed loss fraction on worst landing (nose-into-ground). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingSpeedPenaltyMax = 0.4f;

	/** Seconds of reduced control after a bad landing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingControlPenaltyDuration = 0.5f;

	/** Control multiplier during landing penalty period. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingControlPenaltyFactor = 0.5f;

	/** Dot product above which landing is considered perfect (no penalty). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingQualityThreshold = 0.85f;
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FCameraTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseArmLength = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxArmLength = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float ArmLengthInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BasePitch = -30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedPitch = -22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraHeadingFollow = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraYawInterpSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseFOV = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxFOV = 82.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float FOVInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraTurnLeadWeight = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraLookAheadWeight = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraLookAheadMaxYaw = 20.0f;
};

// --- Equipment & Surface Types ---

USTRUCT(BlueprintType)
struct POWDERRUSH_API FEquipmentStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Equipment")
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Equipment")
	float CarveMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Equipment")
	float TrickSpinMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Equipment")
	float StabilityMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FSurfaceProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	float Friction = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	float CarveGrip = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	float SpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	float SnowSprayAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	FLinearColor SprayColor = FLinearColor(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	FLinearColor SurfaceColor = FLinearColor(0.92f, 0.94f, 0.98f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	float BlendTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	ESurfaceType Type = ESurfaceType::Powder;

	static FSurfaceProperties GetPreset(ESurfaceType SurfaceType)
	{
		FSurfaceProperties P;
		P.Type = SurfaceType;
		switch (SurfaceType)
		{
		case ESurfaceType::Powder:
			P.Friction = 1.0f; P.CarveGrip = 1.0f; P.SpeedMultiplier = 1.0f;
			P.SnowSprayAmount = 1.5f; P.SurfaceColor = FLinearColor(0.92f, 0.94f, 0.98f);
			break;
		case ESurfaceType::Ice:
			P.Friction = 0.3f; P.CarveGrip = 0.4f; P.SpeedMultiplier = 1.15f;
			P.SnowSprayAmount = 0.2f; P.SprayColor = FLinearColor(0.7f, 0.85f, 1.0f);
			P.SurfaceColor = FLinearColor(0.78f, 0.88f, 0.98f);
			break;
		case ESurfaceType::Moguls:
			P.Friction = 1.8f; P.CarveGrip = 0.7f; P.SpeedMultiplier = 0.8f;
			P.SnowSprayAmount = 0.8f; P.SurfaceColor = FLinearColor(0.95f, 0.93f, 0.90f);
			break;
		case ESurfaceType::Groomed:
			P.Friction = 0.7f; P.CarveGrip = 1.3f; P.SpeedMultiplier = 1.05f;
			P.SnowSprayAmount = 0.6f; P.SurfaceColor = FLinearColor(0.85f, 0.90f, 0.98f);
			break;
		}
		return P;
	}
};

// --- Weather ---

UENUM(BlueprintType)
enum class EWeatherPreset : uint8
{
	ClearDay,
	Overcast,
	Snowfall,
	Blizzard,
	Sunset,
	Twilight
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FWeatherConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	EWeatherPreset Preset = EWeatherPreset::ClearDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	FLinearColor SunColor = FLinearColor(1.0f, 0.97f, 0.90f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float SunIntensity = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	FRotator SunRotation = FRotator(-40.0f, -60.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	FLinearColor SkyColor = FLinearColor(0.4f, 0.65f, 0.95f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float SkyLightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float FogDensity = 0.002f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.95f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float FogStartDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float FogHeightFalloff = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float SnowfallRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float WindStrength = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float WindDirection = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Weather")
	float TransitionTime = 5.0f;
};

// --- Procedural Obstacle Params ---

USTRUCT(BlueprintType)
struct POWDERRUSH_API FProceduralTreeParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D TrunkHeightRange = FVector2D(150.0f, 350.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D TrunkRadiusRange = FVector2D(20.0f, 45.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FLinearColor TrunkColor = FLinearColor(0.35f, 0.2f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D FoliageHeightRange = FVector2D(250.0f, 500.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D FoliageRadiusRange = FVector2D(80.0f, 180.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FLinearColor FoliageColor = FLinearColor(0.1f, 0.35f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	int32 MaxFoliageLayers = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	float SnowCapChance = 0.3f;
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FProceduralRockParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D BaseSizeRange = FVector2D(50.0f, 150.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D ScaleXRange = FVector2D(0.6f, 1.4f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D ScaleYRange = FVector2D(0.5f, 1.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FVector2D ScaleZRange = FVector2D(0.3f, 0.9f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FLinearColor BaseColor = FLinearColor(0.4f, 0.4f, 0.42f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	FLinearColor ColorVariation = FLinearColor(0.1f, 0.08f, 0.05f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	float ClusterChance = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	int32 MaxClusterCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Obstacles")
	float SnowCoverChance = 0.4f;
};
