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
