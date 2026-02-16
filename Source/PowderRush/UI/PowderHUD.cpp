#include "UI/PowderHUD.h"
#include "Player/PowderMovementComponent.h"
#include "Core/PowderGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void APowderHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	// Check if we're on the finish/score screen
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->GetRunState() == EPowderRunState::ScoreScreen)
		{
			// Draw centered "FINISHED!" text
			float CenterX = Canvas->SizeX * 0.5f;
			float CenterY = Canvas->SizeY * 0.4f;

			// Dark overlay
			DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

			FString FinishedText = TEXT("FINISHED!");
			float TextW, TextH;
			GetTextSize(FinishedText, TextW, TextH, GEngine->GetLargeFont(), 1.0f);
			DrawText(FinishedText, FColor::White, CenterX - TextW * 0.5f, CenterY - TextH * 0.5f, GEngine->GetLargeFont(), 1.0f);

			FString RestartText = TEXT("Tap to restart");
			float RestartW, RestartH;
			GetTextSize(RestartText, RestartW, RestartH);
			DrawText(RestartText, FColor(200, 200, 200), CenterX - RestartW * 0.5f, CenterY + TextH + 10.0f);

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
	float CarveAngle = MoveComp->GetCarveAngle();
	bool bAirborne = MoveComp->IsAirborne();

	// Layout constants
	float BarWidth = 250.0f;
	float BarHeight = 16.0f;
	float Padding = 20.0f;
	float TextH = 20.0f;
	float Gap = 6.0f;

	// Calculate total panel height
	float PanelHeight = TextH + Gap + BarHeight + Gap + TextH + Gap + BarHeight + Gap + TextH;
	if (bAirborne)
	{
		PanelHeight += Gap + TextH;
	}

	float PanelX = Padding - 6.0f;
	float PanelY = Canvas->SizeY - Padding - PanelHeight - 6.0f;
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
	Y += BarHeight + Gap;

	// Carve angle
	FString CarveText = FString::Printf(TEXT("Carve: %.1f deg"), CarveAngle);
	DrawText(CarveText, FColor(255, 220, 80), Padding, Y);
	Y += TextH;

	// Airborne indicator
	if (bAirborne)
	{
		Y += Gap;
		DrawText(TEXT("AIRBORNE"), FColor(80, 255, 80), Padding, Y);
	}
}
