#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/PowderTypes.h"
#include "PowderAnimInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UPowderAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Animation interface for PowderRush character.
 * Implement on SkeletalMesh-based characters to replace direct mesh rotation in TrickComponent.
 */
class POWDERRUSH_API IPowderAnimInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PowderRush|Animation")
	void PlayTrickAnimation(EPowderTrickType TrickType, float Duration);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PowderRush|Animation")
	void PlayLandAnimation(bool bCleanLanding);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PowderRush|Animation")
	void SetCarveLean(float Lean);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PowderRush|Animation")
	void SetSpeedFactor(float SpeedNorm);
};
