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

protected:
	// Trick notification state
	FString TrickNotificationText;
	float TrickNotificationTimer = 0.0f;
	float TrickNotificationDuration = 1.5f;

	UFUNCTION()
	void OnTrickCompleted(EPowderTrickType TrickType, int32 Points);

	UFUNCTION()
	void OnTrickFailed();
};
