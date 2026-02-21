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

	/** World gravity in cm/s^2. Default 980 = Earth gravity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GravityAcceleration = 980.0f;

	/** Fallback slope angle (degrees) when no terrain normal is available. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SlopeAngle = 15.0f;

	/** Scales gravity along slope. Lower = slower acceleration, higher = steeper feel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GravityAlongSlopeScale = 0.45f;

	/** Caps slope angle for acceleration. Prevents runaway speed on very steep terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxAccelerationSlopeAngle = 28.0f;

	/** Top speed in cm/s. Higher = faster runs, harder to control. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxSpeed = 3000.0f;

	/** Base snow friction. Higher = slower overall, more drag when straight-lining. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseFriction = 0.02f;

	/** Speed penalty from carving. Higher = more speed lost in turns. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveSpeedBleed = 0.5f;

	/** How fast speed bleed ramps up/down. Lower = smoother transitions, higher = snappier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveBleedSmoothing = 1.5f;

	/** How fast the carve angle moves toward target. Higher = snappier turn entry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRate = 3.0f;

	/** How fast the carve angle returns to center on release. Lower = longer arcs after release. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveReturnRate = 0.5f;

	/** Maximum carve deflection in degrees. Wider = more extreme turns possible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxCarveAngle = 90.0f;

	/** Base yaw rotation speed in deg/s. Legacy param, mostly superseded by TurnRateLimit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float YawRate = 90.0f;

	/** Sideways drift speed during carves in cm/s. Creates the carved arc movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveLateralSpeed = 1400.0f;

	/** Rate boost meter fills from carving (0-1 per second). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostFillRate = 0.3f;

	/** Extra speed added during boost in cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostBurstSpeed = 1500.0f;

	/** How long boost lasts in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BoostDuration = 0.5f;

	/** Upward impulse for ollie in cm/s. Higher = bigger air. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float OllieForce = 450.0f;

	/** Minimum seconds between ollies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float OllieCooldown = 1.0f;

	/** FInterpTo speed for raw input smoothing. Higher = more responsive, lower = smoother. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveInputSmoothing = 10.0f;

	/** Seconds for touch input to ramp from min to full intensity. Lower = snappier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampTime = 0.25f;

	/** Instant carve intensity on first touch frame (0-1). Higher = more immediate response. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampMinIntensity = 0.5f;

	/** Ease-in curve shape for ramp. 1.0 = linear, 2.0 = quadratic, lower = faster ramp. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveRampEaseExponent = 1.5f;

	/** How much speed limits max carve angle. 0 = no limit, 1 = full limit at max speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnLimitFactor = 0.5f;

	/** Minimum allowed carve angle at max speed (degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MinTurnAngleAtMaxSpeed = 30.0f;

	/** Visual yaw lag in seconds. 0 = instant, >0 = capsule rotation lags behind heading. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float YawSmoothing = 0.0f;

	/** Minimum Z component of ground normal to count as walkable terrain. Rejects walls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MinGroundNormalZ = 0.2f;

	/** FInterpTo speed for filtering terrain normal. Higher = tracks bumps, lower = smooths them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float GroundNormalFilterSpeed = 12.0f;

	/** Deg/s the heading auto-aligns back to downhill when input is neutral. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float DownhillAlignRate = 55.0f;

	/** Max heading change rate in deg/s. Caps how fast you can steer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnRateLimitDegPerSec = 220.0f;

	/** Exponent for carve-depth-to-speed-bleed curve. Higher = less bleed at shallow angles. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarveBleedExponent = 1.8f;

	// --- Ski Feel ---

	/** Turn rate multiplier at max speed. 0.35 = 35% of base turn rate when going full speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnRateMin = 0.35f;

	/** How aggressively turn rate drops with speed. >1 = drops faster at high speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedTurnRateExponent = 1.2f;

	/** FInterpTo rate for edge engagement. Higher = edges bite faster on input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeEngageRate = 6.0f;

	/** FInterpTo rate for edge release. Higher = edges release faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeDisengageRate = 4.0f;

	/** Minimum edge depth on input so taps produce immediate response. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeMinDepth = 0.3f;

	/** Gravity factor at 90 degrees off fall line. 0 = coast, 1 = full gravity, <0 = brake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingTraverseFactor = 0.15f;

	/** Gravity factor heading uphill. Negative = active braking. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingUphillFactor = -0.3f;

	/** Friction scaling off fall line. Lower = less drag when traversing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float HeadingFrictionScale = 0.5f;

	/** Seconds the carve arc holds after input release. Longer = more momentum carry. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnCommitTime = 0.2f;

	/** How much the committed arc decays. 0 = hold exact angle, 1 = fade to straight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TurnCommitDecay = 0.3f;

	/** Max speed loss on worst landing (0-1). 0.4 = lose up to 40% speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingSpeedPenaltyMax = 0.4f;

	/** Seconds of reduced steering after a bad landing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingControlPenaltyDuration = 0.5f;

	/** Steering multiplier during landing penalty. 0.5 = half steering authority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingControlPenaltyFactor = 0.5f;

	/** Landing quality above this threshold = perfect (no penalty). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingQualityThreshold = 0.85f;

	// --- Terrain & Air ---

	/** Height difference below which terrain snap is skipped to prevent micro-jitter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float TerrainSnapThreshold = 3.0f;

	/** Seconds to blend from airborne to full ground physics after landing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float LandingBlendDuration = 0.15f;

	/** Air resistance. Damps all velocity axes in air. 0 = none, 1 = heavy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float AirDragCoefficient = 0.5f;

	/** Max downward speed in air (cm/s). Prevents infinite freefall acceleration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float AirTerminalVelocity = 1500.0f;

	// --- Edge Feel ---

	/** Seconds of low-grip flat-ski phase when switching edges (left to right). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeTransitionTime = 0.06f;

	/** Grip multiplier during edge transition. 0.15 = 15% grip, near-zero = slippery. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float EdgeTransitionGrip = 0.15f;

	/** Rate carve pressure builds during sustained holds (0 to 1 in ~0.7s at 1.5). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarvePressureBuildRate = 1.5f;

	/** Rate carve pressure drops on release. Higher = pressure vanishes faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarvePressureDecayRate = 3.0f;

	/** Extra turn rate at max pressure. 0.15 = 15% faster turning when fully committed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarvePressureTurnBonus = 0.15f;

	/** Extra speed bleed at max pressure. Deep committed carves cost more speed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CarvePressureBleedBonus = 0.2f;
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FCameraTuning
{
	GENERATED_BODY()

	/** Spring arm length at rest (cm). Larger = more zoomed out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseArmLength = 1400.0f;

	/** Spring arm length at max speed (cm). Creates a zoom-out effect as speed increases. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxArmLength = 1800.0f;

	/** How fast the arm length interpolates between Base and Max (higher = snappier zoom). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float ArmLengthInterpSpeed = 3.0f;

	/** Camera pitch at rest (degrees, negative = looking down). Controls the default viewing angle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BasePitch = -30.0f;

	/** Camera pitch at max speed (degrees). Typically flatter than base to show more ahead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float SpeedPitch = -22.0f;

	/** Static yaw offset from behind the character (degrees). 0 = directly behind. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseYawOffset = 0.0f;

	/** How much the camera yaw tracks the character's heading direction (0=none, 1=fully follows). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraHeadingFollow = 0.5f;

	/** Smoothing speed for camera yaw rotation (lower = more lag, higher = snappier follow). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraYawInterpSpeed = 0.5f;

	/** Field of view at rest (degrees). Standard is 70-75. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float BaseFOV = 72.0f;

	/** Field of view at max speed (degrees). Creates a speed rush effect as FOV widens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float MaxFOV = 82.0f;

	/** How fast FOV interpolates between Base and Max (higher = snappier response). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float FOVInterpSpeed = 3.0f;

	/** How much the camera leads into turns (0=none, 1=full). Anticipates carve direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraTurnLeadWeight = 0.55f;

	/** How much the camera looks ahead of the character's velocity (0=none, 1=full). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tuning")
	float CameraLookAheadWeight = 0.35f;

	/** Maximum yaw angle for look-ahead offset (degrees). Caps how far the camera can swing. */
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
