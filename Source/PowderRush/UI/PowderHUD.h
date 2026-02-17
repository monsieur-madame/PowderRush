#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/PowderTypes.h"
#include "PowderHUD.generated.h"

struct FLifetimeStats;

USTRUCT()
struct FScreenButton
{
	GENERATED_BODY()

	FString Label;
	FVector2D Pos = FVector2D::ZeroVector;
	FVector2D Size = FVector2D::ZeroVector;
	FName Action;
};

UCLASS()
class POWDERRUSH_API APowderHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

	void ShowPowerupIndicator(EPowerupType Type, float Duration);

	/** Called by controller to handle menu taps. Returns true if a button was hit. */
	bool OnMenuTap(float X, float Y);

	/** Check if tap is in the pause button region (top-left). */
	bool IsPauseAreaHit(float X, float Y) const;

protected:
	// Button system
	TArray<FScreenButton> ActiveButtons;
	void ClearButtons();
	void AddButton(const FString& Label, FVector2D Pos, FVector2D Size, FName Action);
	FName TestButtonHit(float X, float Y) const;
	void DrawButton(const FString& Label, FVector2D Pos, FVector2D Size, FColor TextColor, FLinearColor BgColor);

	// Menu drawing
	void DrawMainMenu();
	void DrawPauseMenu();
	void DrawStatsScreen();
	void DrawScoreScreen();
	void DrawGameplayHUD(float DeltaTime);

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

	// Stats screen toggle
	bool bShowingStats = false;

	UFUNCTION()
	void OnTrickCompleted(EPowderTrickType TrickType, int32 Points);

	UFUNCTION()
	void OnTrickFailed();
};
