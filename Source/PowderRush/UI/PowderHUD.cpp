#include "UI/PowderHUD.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "Engine/GameInstance.h"
#include "Core/PowderGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void APowderHUD::BeginPlay()
{
	Super::BeginPlay();

	// Bind to trick component events
	if (APawn* Pawn = GetOwningPawn())
	{
		if (UPowderTrickComponent* TrickComp = Pawn->FindComponentByClass<UPowderTrickComponent>())
		{
			TrickComp->OnTrickCompleted.AddDynamic(this, &APowderHUD::OnTrickCompleted);
			TrickComp->OnTrickFailed.AddDynamic(this, &APowderHUD::OnTrickFailed);
		}
	}
}

void APowderHUD::ShowPowerupIndicator(EPowerupType Type, float Duration)
{
	bPowerupActive = true;
	ActivePowerupType = Type;
	PowerupTimeRemaining = Duration;
	PowerupDuration = Duration;

	// Set flash text
	switch (Type)
	{
	case EPowerupType::SpeedBoost:
		PowerupFlashText = TEXT("SPEED BOOST!");
		break;
	case EPowerupType::ScoreMultiplier:
		PowerupFlashText = TEXT("2x SCORE!");
		break;
	}
	PowerupFlashTimer = PowerupFlashDuration;
}

void APowderHUD::OnTrickCompleted(EPowderTrickType TrickType, int32 Points)
{
	// Map trick type to display name
	switch (TrickType)
	{
	case EPowderTrickType::Backflip:      TrickNotificationText = FString::Printf(TEXT("BACKFLIP! +%d"), Points); break;
	case EPowderTrickType::Frontflip:     TrickNotificationText = FString::Printf(TEXT("FRONTFLIP! +%d"), Points); break;
	case EPowderTrickType::HeliSpinLeft:  TrickNotificationText = FString::Printf(TEXT("HELI SPIN! +%d"), Points); break;
	case EPowderTrickType::HeliSpinRight: TrickNotificationText = FString::Printf(TEXT("HELI SPIN! +%d"), Points); break;
	case EPowderTrickType::SpreadEagle:   TrickNotificationText = FString::Printf(TEXT("SPREAD EAGLE! +%d"), Points); break;
	default:                              TrickNotificationText = FString::Printf(TEXT("TRICK! +%d"), Points); break;
	}
	TrickNotificationTimer = TrickNotificationDuration;
}

void APowderHUD::OnTrickFailed()
{
	TrickNotificationText = TEXT("WIPEOUT!");
	TrickNotificationTimer = TrickNotificationDuration;
}

void APowderHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// Tick timers
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (TrickNotificationTimer > 0.0f)
	{
		TrickNotificationTimer -= DeltaTime;
	}
	if (PowerupFlashTimer > 0.0f)
	{
		PowerupFlashTimer -= DeltaTime;
	}
	if (bPowerupActive)
	{
		PowerupTimeRemaining -= DeltaTime;
		if (PowerupTimeRemaining <= 0.0f)
		{
			bPowerupActive = false;
			PowerupTimeRemaining = 0.0f;
		}
	}

	// Check if we're on the finish/score screen
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GetRunState() == EPowderRunState::ScoreScreen)
		{
			// Dark overlay
			DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

			float CenterX = Canvas->SizeX * 0.5f;
			float CenterY = Canvas->SizeY * 0.4f;

			FString FinishedText = TEXT("FINISHED!");
			float TextW, TextH;
			GetTextSize(FinishedText, TextW, TextH, GEngine->GetLargeFont(), 1.0f);
			DrawText(FinishedText, FColor::White, CenterX - TextW * 0.5f, CenterY - TextH * 0.5f, GEngine->GetLargeFont(), 1.0f);

			// Show final score
			if (UScoreSubsystem* ScoreSys = GetOwningPlayerController()->GetGameInstance()->GetSubsystem<UScoreSubsystem>())
			{
				FString ScoreText = FString::Printf(TEXT("Score: %d"), ScoreSys->GetCurrentScore());
				float ScoreW, ScoreH;
				GetTextSize(ScoreText, ScoreW, ScoreH, GEngine->GetLargeFont(), 1.0f);
				DrawText(ScoreText, FColor(255, 220, 80), CenterX - ScoreW * 0.5f, CenterY + TextH + 5.0f, GEngine->GetLargeFont(), 1.0f);
			}

			FString RestartText = TEXT("Tap to restart");
			float RestartW, RestartH;
			GetTextSize(RestartText, RestartW, RestartH);
			DrawText(RestartText, FColor(200, 200, 200), CenterX - RestartW * 0.5f, CenterY + 60.0f);

			return;
		}
	}

	APawn* Pawn = GetOwningPawn();
	if (!Pawn)
	{
		return;
	}

	UPowderMovementComponent* MoveComp = Pawn->FindComponentByClass<UPowderMovementComponent>();
	if (!MoveComp)
	{
		return;
	}

	float Speed = MoveComp->GetCurrentSpeed();
	float SpeedNorm = MoveComp->GetSpeedNormalized();
	float Boost = MoveComp->GetBoostMeter();
	bool bAirborne = MoveComp->IsAirborne();

	// --- Score display (top-center) ---
	UScoreSubsystem* ScoreSys = GetOwningPlayerController()
		? GetOwningPlayerController()->GetGameInstance()->GetSubsystem<UScoreSubsystem>()
		: nullptr;

	if (ScoreSys)
	{
		float CenterX = Canvas->SizeX * 0.5f;
		float ScoreY = 20.0f;

		// Score
		FString ScoreText = FString::Printf(TEXT("%d"), ScoreSys->GetCurrentScore());
		float ScoreW, ScoreH;
		GetTextSize(ScoreText, ScoreW, ScoreH, GEngine->GetLargeFont(), 1.0f);
		DrawText(ScoreText, FColor::White, CenterX - ScoreW * 0.5f, ScoreY, GEngine->GetLargeFont(), 1.0f);

		// Multiplier (only when > 1.0x)
		float Multiplier = ScoreSys->GetMultiplier();
		if (Multiplier > 1.0f)
		{
			FString MultText = FString::Printf(TEXT("x%.1f"), Multiplier);
			float MultW, MultH;
			GetTextSize(MultText, MultW, MultH, GEngine->GetLargeFont(), 1.0f);
			DrawText(MultText, FColor(255, 180, 0), CenterX - MultW * 0.5f, ScoreY + ScoreH + 2.0f, GEngine->GetLargeFont(), 1.0f);
		}

		// Combo timer bar (below score)
		float ComboNorm = ScoreSys->GetComboTimerNormalized();
		if (ComboNorm > 0.0f)
		{
			float ComboBarWidth = 150.0f;
			float ComboBarHeight = 4.0f;
			float ComboBarX = CenterX - ComboBarWidth * 0.5f;
			float ComboBarY = ScoreY + ScoreH + 24.0f;

			DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.6f), ComboBarX, ComboBarY, ComboBarWidth, ComboBarHeight);
			DrawRect(FLinearColor(1.0f, 0.7f, 0.0f, 1.0f), ComboBarX, ComboBarY, ComboBarWidth * ComboNorm, ComboBarHeight);
		}
	}

	// --- Powerup indicator (top-right area) ---
	if (bPowerupActive)
	{
		float IndicatorX = Canvas->SizeX - 200.0f;
		float IndicatorY = 20.0f;
		float BarWidth = 160.0f;
		float BarHeight = 8.0f;

		// Color based on type: blue for speed, gold for score
		FColor LabelColor;
		FLinearColor BarColor;
		FString LabelText;
		switch (ActivePowerupType)
		{
		case EPowerupType::SpeedBoost:
			LabelColor = FColor(80, 180, 255);
			BarColor = FLinearColor(0.3f, 0.7f, 1.0f);
			LabelText = TEXT("SPEED BOOST");
			break;
		case EPowerupType::ScoreMultiplier:
			LabelColor = FColor(255, 220, 80);
			BarColor = FLinearColor(1.0f, 0.85f, 0.3f);
			LabelText = TEXT("2x SCORE");
			break;
		}

		DrawText(LabelText, LabelColor, IndicatorX, IndicatorY);

		// Timer bar
		float TimerNorm = (PowerupDuration > 0.0f) ? FMath::Clamp(PowerupTimeRemaining / PowerupDuration, 0.0f, 1.0f) : 0.0f;
		float TimerBarY = IndicatorY + 20.0f;
		DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.6f), IndicatorX, TimerBarY, BarWidth, BarHeight);
		DrawRect(BarColor, IndicatorX, TimerBarY, BarWidth * TimerNorm, BarHeight);
	}

	// --- Powerup flash text (center screen) ---
	if (PowerupFlashTimer > 0.0f)
	{
		float CenterX = Canvas->SizeX * 0.5f;
		float FlashY = Canvas->SizeY * 0.28f;

		float Alpha = FMath::Clamp(PowerupFlashTimer / 0.3f, 0.0f, 1.0f);

		FColor FlashColor;
		switch (ActivePowerupType)
		{
		case EPowerupType::SpeedBoost:
			FlashColor = FColor(80, 180, 255, FMath::RoundToInt32(Alpha * 255));
			break;
		case EPowerupType::ScoreMultiplier:
			FlashColor = FColor(255, 220, 80, FMath::RoundToInt32(Alpha * 255));
			break;
		}

		float FlashW, FlashH;
		GetTextSize(PowerupFlashText, FlashW, FlashH, GEngine->GetLargeFont(), 1.0f);
		DrawText(PowerupFlashText, FlashColor,
			CenterX - FlashW * 0.5f, FlashY - FlashH * 0.5f,
			GEngine->GetLargeFont(), 1.0f);
	}

	// --- Trick notification (center screen, brief flash) ---
	if (TrickNotificationTimer > 0.0f)
	{
		float CenterX = Canvas->SizeX * 0.5f;
		float CenterY = Canvas->SizeY * 0.35f;

		float Alpha = FMath::Clamp(TrickNotificationTimer / 0.3f, 0.0f, 1.0f);
		float Scale = 1.0f + (1.0f - Alpha) * 0.3f; // Slight scale-up as it fades

		float TrickW, TrickH;
		GetTextSize(TrickNotificationText, TrickW, TrickH, GEngine->GetLargeFont(), 1.0f);

		// Pick color: wipeout = red, tricks = yellow/green
		FColor TrickColor = TrickNotificationText.Contains(TEXT("WIPEOUT"))
			? FColor(255, 80, 80, FMath::RoundToInt32(Alpha * 255))
			: FColor(255, 255, 80, FMath::RoundToInt32(Alpha * 255));

		DrawText(TrickNotificationText, TrickColor,
			CenterX - TrickW * 0.5f, CenterY - TrickH * 0.5f,
			GEngine->GetLargeFont(), 1.0f);
	}

	// --- Bottom-left debug panel ---
	float BarWidth = 250.0f;
	float BarHeight = 16.0f;
	float Padding = 20.0f;
	float TextH = 20.0f;
	float Gap = 6.0f;

	float PanelHeight = TextH + Gap + BarHeight + Gap + TextH + Gap + BarHeight;
	if (bAirborne)
	{
		PanelHeight += Gap + TextH;
	}

	float PanelX = Padding - 6.0f;
	float PanelY = Canvas->SizeY - Padding - PanelHeight - 12.0f;
	float PanelW = BarWidth + 12.0f;

	// Dark background panel
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f), PanelX, PanelY, PanelW, PanelHeight + 12.0f);

	float Y = PanelY + 6.0f;

	// Speed text
	FString SpeedText = FString::Printf(TEXT("Speed: %.0f / %.0f  (%.0f%%)"),
		Speed, MoveComp->MaxSpeed, SpeedNorm * 100.0f);
	DrawText(SpeedText, FColor::White, Padding, Y);
	Y += TextH + Gap;

	// Speed bar
	DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f), Padding, Y, BarWidth, BarHeight);
	FLinearColor SpeedColor = FLinearColor::LerpUsingHSV(
		FLinearColor(0.2f, 0.8f, 0.2f), FLinearColor(1.0f, 0.2f, 0.1f), SpeedNorm);
	DrawRect(SpeedColor, Padding, Y, BarWidth * SpeedNorm, BarHeight);
	Y += BarHeight + Gap;

	// Boost text
	FString BoostText = FString::Printf(TEXT("Boost: %.0f%%"), Boost * 100.0f);
	DrawText(BoostText, FColor(100, 180, 255), Padding, Y);
	Y += TextH + Gap;

	// Boost bar
	DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f), Padding, Y, BarWidth, BarHeight);
	DrawRect(FLinearColor(0.2f, 0.6f, 1.0f), Padding, Y, BarWidth * Boost, BarHeight);
	Y += BarHeight;

	// Airborne indicator
	if (bAirborne)
	{
		Y += Gap;
		DrawText(TEXT("AIRBORNE"), FColor(80, 255, 80), Padding, Y);
	}
}
