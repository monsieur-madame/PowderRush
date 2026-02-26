#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PowderAnimInstance.generated.h"

UCLASS()
class POWDERRUSH_API UPowderAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	float CarveLean = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	float EdgeDepth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	float SpeedNorm = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	float SlopePitch = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	bool bAirborne = false;

	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Animation")
	bool bCarving = false;
};
