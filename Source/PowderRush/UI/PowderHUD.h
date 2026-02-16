#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/PowderTypes.h"
#include "PowderHUD.generated.h"

UCLASS()
class POWDERRUSH_API APowderHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	void ShowPowerupIndicator(EPowerupType Type, float Duration);

protected:
	// Powerup indicator
	bool bPowerupActive = false;
	EPowerupType ActivePowerupType = EPowerupType::SpeedBoost;
	float PowerupTimeRemaining = 0.0f;
	float PowerupDuration = 0.0f;

	// Powerup flash text
	FString PowerupFlashText;
	float PowerupFlashTimer = 0.0f;
	static constexpr float PowerupFlashDuration = 1.2f;

	// Trick notification state
	FString TrickNotificationText;
	float TrickNotificationTimer = 0.0f;
	float TrickNotificationDuration = 1.5f;

	UFUNCTION()
	void OnTrickCompleted(EPowderTrickType TrickType, int32 Points);

	UFUNCTION()
	void OnTrickFailed();
};
