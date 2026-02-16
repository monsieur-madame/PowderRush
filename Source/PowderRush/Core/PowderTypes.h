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

UENUM(BlueprintType)
enum class EZoneType : uint8
{
	PowderBowl,
	TreeSlalom,
	IceSheet,
	MogulField,
	CliffRun,
	JumpPark
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
	HoldBoth
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
	float SnowSprayAmount = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Surface")
	ESurfaceType Type = ESurfaceType::Powder;
};
