#include "UI/PowderHUD.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "Core/PowderGameMode.h"
#include "Core/PowderGameInstance.h"
#include "Meta/PowderSaveGame.h"
#include "Engine/GameInstance.h"
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

// --- Button System ---

void APowderHUD::ClearButtons()
{
	ActiveButtons.Empty();
}

void APowderHUD::AddButton(const FString& Label, FVector2D Pos, FVector2D Size, FName Action)
{
	FScreenButton Btn;
	Btn.Label = Label;
	Btn.Pos = Pos;
	Btn.Size = Size;
	Btn.Action = Action;
	ActiveButtons.Add(Btn);
}

FName APowderHUD::TestButtonHit(float X, float Y) const
{
	for (const FScreenButton& Btn : ActiveButtons)
	{
		if (X >= Btn.Pos.X && X <= Btn.Pos.X + Btn.Size.X &&
			Y >= Btn.Pos.Y && Y <= Btn.Pos.Y + Btn.Size.Y)
		{
			return Btn.Action;
		}
	}
	return NAME_None;
}

void APowderHUD::DrawButton(const FString& Label, FVector2D Pos, FVector2D Size, FColor TextColor, FLinearColor BgColor)
{
	DrawRect(BgColor, Pos.X, Pos.Y, Size.X, Size.Y);

	float TextW, TextH;
	GetTextSize(Label, TextW, TextH, GEngine->GetLargeFont(), 1.0f);
	DrawText(Label, TextColor,
		Pos.X + (Size.X - TextW) * 0.5f,
		Pos.Y + (Size.Y - TextH) * 0.5f,
		GEngine->GetLargeFont(), 1.0f);
}

bool APowderHUD::OnMenuTap(float X, float Y)
{
	FName Action = TestButtonHit(X, Y);
	if (Action == NAME_None)
	{
		return false;
	}

	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		return false;
	}

	if (Action == FName(TEXT("Play")))
	{
		bShowingStats = false;
		GM->RestartRun();
	}
	else if (Action == FName(TEXT("Stats")))
	{
		bShowingStats = true;
	}
	else if (Action == FName(TEXT("StatsBack")))
	{
		bShowingStats = false;
	}
	else if (Action == FName(TEXT("Resume")))
	{
		GM->ResumeRun();
	}
	else if (Action == FName(TEXT("Restart")))
	{
		GM->RestartRun();
	}
	else if (Action == FName(TEXT("QuitToMenu")))
	{
		GM->QuitToMenu();
	}
	else if (Action == FName(TEXT("ScoreMenu")))
	{
		GM->QuitToMenu();
	}
	else if (Action == FName(TEXT("ScoreRestart")))
	{
		GM->RestartRun();
	}

	return true;
}

bool APowderHUD::IsPauseAreaHit(float X, float Y) const
{
	return X <= 80.0f && Y <= 80.0f;
}

// --- DrawHUD ---

void APowderHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Tick timers regardless of state
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

	// Clear buttons each frame — menus re-register them
	ClearButtons();

	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		return;
	}

	switch (GM->GetRunState())
	{
	case EPowderRunState::InMenu:
		if (bShowingStats)
		{
			DrawStatsScreen();
		}
		else
		{
			DrawMainMenu();
		}
		break;

	case EPowderRunState::Paused:
		DrawGameplayHUD(DeltaTime);
		DrawPauseMenu();
		break;

	case EPowderRunState::ScoreScreen:
		DrawScoreScreen();
		break;

	case EPowderRunState::Running:
	case EPowderRunState::WipedOut:
	case EPowderRunState::Starting:
	default:
		DrawGameplayHUD(DeltaTime);
		break;
	}
}

// --- Main Menu ---

void APowderHUD::DrawMainMenu()
{
	// Dark background
	DrawRect(FLinearColor(0.05f, 0.08f, 0.15f, 0.9f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	float CenterX = Canvas->SizeX * 0.5f;
	float TitleY = Canvas->SizeY * 0.2f;

	// Title
	FString Title = TEXT("POWDER RUSH");
	float TitleW, TitleH;
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), 1.0f);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, TitleY, GEngine->GetLargeFont(), 1.0f);

	// Subtitle
	FString Subtitle = TEXT("Arcade Skiing");
	float SubW, SubH;
	GetTextSize(Subtitle, SubW, SubH);
	DrawText(Subtitle, FColor(180, 200, 230), CenterX - SubW * 0.5f, TitleY + TitleH + 8.0f);

	// Play button
	FVector2D BtnSize(220.0f, 60.0f);
	float BtnY = Canvas->SizeY * 0.45f;
	FVector2D PlayPos(CenterX - BtnSize.X * 0.5f, BtnY);
	DrawButton(TEXT("PLAY"), PlayPos, BtnSize, FColor::White, FLinearColor(0.1f, 0.5f, 0.2f, 0.9f));
	AddButton(TEXT("PLAY"), PlayPos, BtnSize, FName(TEXT("Play")));

	// Stats button
	float StatsY = BtnY + BtnSize.Y + 20.0f;
	FVector2D StatsPos(CenterX - BtnSize.X * 0.5f, StatsY);
	DrawButton(TEXT("STATS"), StatsPos, BtnSize, FColor::White, FLinearColor(0.2f, 0.3f, 0.5f, 0.9f));
	AddButton(TEXT("STATS"), StatsPos, BtnSize, FName(TEXT("Stats")));

	// High score
	if (UPowderGameInstance* GI = Cast<UPowderGameInstance>(GetOwningPlayerController()->GetGameInstance()))
	{
		if (GI->GetHighScore() > 0)
		{
			FString HighScoreText = FString::Printf(TEXT("High Score: %d"), GI->GetHighScore());
			float HSW, HSH;
			GetTextSize(HighScoreText, HSW, HSH);
			DrawText(HighScoreText, FColor(255, 220, 80), CenterX - HSW * 0.5f, StatsY + BtnSize.Y + 30.0f);
		}
	}
}

// --- Pause Menu ---

void APowderHUD::DrawPauseMenu()
{
	// Dark overlay
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	float CenterX = Canvas->SizeX * 0.5f;
	float TitleY = Canvas->SizeY * 0.25f;

	FString Title = TEXT("PAUSED");
	float TitleW, TitleH;
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), 1.0f);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, TitleY, GEngine->GetLargeFont(), 1.0f);

	FVector2D BtnSize(220.0f, 55.0f);
	float BtnX = CenterX - BtnSize.X * 0.5f;
	float BtnY = Canvas->SizeY * 0.4f;
	float Gap = 15.0f;

	// Resume
	FVector2D ResumePos(BtnX, BtnY);
	DrawButton(TEXT("RESUME"), ResumePos, BtnSize, FColor::White, FLinearColor(0.1f, 0.5f, 0.2f, 0.9f));
	AddButton(TEXT("RESUME"), ResumePos, BtnSize, FName(TEXT("Resume")));

	// Restart
	FVector2D RestartPos(BtnX, BtnY + BtnSize.Y + Gap);
	DrawButton(TEXT("RESTART"), RestartPos, BtnSize, FColor::White, FLinearColor(0.5f, 0.4f, 0.1f, 0.9f));
	AddButton(TEXT("RESTART"), RestartPos, BtnSize, FName(TEXT("Restart")));

	// Quit to Menu
	FVector2D QuitPos(BtnX, BtnY + (BtnSize.Y + Gap) * 2.0f);
	DrawButton(TEXT("QUIT TO MENU"), QuitPos, BtnSize, FColor::White, FLinearColor(0.5f, 0.15f, 0.1f, 0.9f));
	AddButton(TEXT("QUIT TO MENU"), QuitPos, BtnSize, FName(TEXT("QuitToMenu")));
}

// --- Stats Screen ---

void APowderHUD::DrawStatsScreen()
{
	// Dark background
	DrawRect(FLinearColor(0.05f, 0.08f, 0.15f, 0.9f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	float CenterX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.08f;

	FString Title = TEXT("LIFETIME STATS");
	float TitleW, TitleH;
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), 1.0f);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, Y, GEngine->GetLargeFont(), 1.0f);
	Y += TitleH + 20.0f;

	UPowderGameInstance* GI = Cast<UPowderGameInstance>(GetOwningPlayerController()->GetGameInstance());
	if (GI)
	{
		const FLifetimeStats& Stats = GI->GetLifetimeStats();
		float LeftX = Canvas->SizeX * 0.1f;
		float LineH = 22.0f;

		auto DrawStat = [&](const FString& Label, const FString& Value)
		{
			DrawText(Label, FColor(180, 200, 220), LeftX, Y);
			float ValW, ValH;
			GetTextSize(Value, ValW, ValH);
			DrawText(Value, FColor(255, 220, 80), Canvas->SizeX * 0.9f - ValW, Y);
			Y += LineH;
		};

		DrawStat(TEXT("Total Runs"), FString::Printf(TEXT("%d"), Stats.TotalRuns));
		DrawStat(TEXT("Best Score"), FString::Printf(TEXT("%d"), Stats.BestScore));
		DrawStat(TEXT("Best Distance"), FString::Printf(TEXT("%.0f m"), Stats.BestDistance / 100.0f));
		DrawStat(TEXT("Best Combo Chain"), FString::Printf(TEXT("%d"), Stats.BestComboChain));
		DrawStat(TEXT("Highest Multiplier"), FString::Printf(TEXT("x%.1f"), Stats.HighestMultiplier));
		DrawStat(TEXT("Longest Air Time"), FString::Printf(TEXT("%.1f s"), Stats.LongestAirTime));
		DrawStat(TEXT("Most Tricks in Run"), FString::Printf(TEXT("%d"), Stats.MostTricksInRun));
		DrawStat(TEXT("Most Near Misses"), FString::Printf(TEXT("%d"), Stats.MostNearMissesInRun));

		Y += 10.0f;
		DrawText(TEXT("--- Lifetime Totals ---"), FColor(140, 160, 180), LeftX, Y);
		Y += LineH;

		DrawStat(TEXT("Total Coins Earned"), FString::Printf(TEXT("%d"), Stats.TotalCoinsEarned));
		DrawStat(TEXT("Total Tricks Landed"), FString::Printf(TEXT("%d"), Stats.TotalTricksLanded));
		DrawStat(TEXT("Total Near Misses"), FString::Printf(TEXT("%d"), Stats.TotalNearMisses));
		DrawStat(TEXT("Total Gates Passed"), FString::Printf(TEXT("%d"), Stats.TotalGatesPassed));
		DrawStat(TEXT("Total Distance"), FString::Printf(TEXT("%.0f m"), Stats.TotalDistanceTraveled / 100.0f));
	}

	// Back button
	FVector2D BtnSize(180.0f, 50.0f);
	FVector2D BackPos(CenterX - BtnSize.X * 0.5f, Canvas->SizeY * 0.88f);
	DrawButton(TEXT("BACK"), BackPos, BtnSize, FColor::White, FLinearColor(0.3f, 0.3f, 0.4f, 0.9f));
	AddButton(TEXT("BACK"), BackPos, BtnSize, FName(TEXT("StatsBack")));
}

// --- Score Screen ---

void APowderHUD::DrawScoreScreen()
{
	// Dark overlay
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	float CenterX = Canvas->SizeX * 0.5f;
	float Y = Canvas->SizeY * 0.15f;

	FString FinishedText = TEXT("FINISHED!");
	float TextW, TextH;
	GetTextSize(FinishedText, TextW, TextH, GEngine->GetLargeFont(), 1.0f);
	DrawText(FinishedText, FColor::White, CenterX - TextW * 0.5f, Y, GEngine->GetLargeFont(), 1.0f);
	Y += TextH + 15.0f;

	UScoreSubsystem* ScoreSys = GetOwningPlayerController()
		? GetOwningPlayerController()->GetGameInstance()->GetSubsystem<UScoreSubsystem>()
		: nullptr;
	UPowderGameInstance* GI = Cast<UPowderGameInstance>(GetOwningPlayerController()->GetGameInstance());

	if (ScoreSys)
	{
		const FRunStats& RunStats = ScoreSys->GetCurrentRunStats();

		// Score
		FString ScoreText = FString::Printf(TEXT("Score: %d"), RunStats.TotalScore);
		float ScoreW, ScoreH;
		GetTextSize(ScoreText, ScoreW, ScoreH, GEngine->GetLargeFont(), 1.0f);
		DrawText(ScoreText, FColor(255, 220, 80), CenterX - ScoreW * 0.5f, Y, GEngine->GetLargeFont(), 1.0f);
		Y += ScoreH + 5.0f;

		// New best indicator
		if (GI && RunStats.TotalScore >= GI->GetHighScore() && RunStats.TotalScore > 0)
		{
			FString BestText = TEXT("NEW BEST!");
			float BestW, BestH;
			GetTextSize(BestText, BestW, BestH, GEngine->GetLargeFont(), 1.0f);
			DrawText(BestText, FColor(80, 255, 80), CenterX - BestW * 0.5f, Y, GEngine->GetLargeFont(), 1.0f);
			Y += BestH + 5.0f;
		}

		Y += 10.0f;
		float LeftX = Canvas->SizeX * 0.2f;
		float LineH = 20.0f;

		auto DrawRunStat = [&](const FString& Label, const FString& Value)
		{
			DrawText(Label, FColor(200, 200, 200), LeftX, Y);
			float ValW, ValH;
			GetTextSize(Value, ValW, ValH);
			DrawText(Value, FColor::White, Canvas->SizeX * 0.8f - ValW, Y);
			Y += LineH;
		};

		DrawRunStat(TEXT("Distance"), FString::Printf(TEXT("%.0f m"), RunStats.TotalDistance / 100.0f));
		DrawRunStat(TEXT("Tricks Landed"), FString::Printf(TEXT("%d"), RunStats.TricksLanded));
		DrawRunStat(TEXT("Near Misses"), FString::Printf(TEXT("%d"), RunStats.NearMissCount));
		DrawRunStat(TEXT("Best Combo"), FString::Printf(TEXT("%d"), RunStats.BestComboChain));
		DrawRunStat(TEXT("Highest Multiplier"), FString::Printf(TEXT("x%.1f"), RunStats.HighestMultiplier));
		DrawRunStat(TEXT("Coins"), FString::Printf(TEXT("%d"), RunStats.CoinsCollected));
	}

	// Buttons
	FVector2D BtnSize(200.0f, 50.0f);
	float BtnY = Canvas->SizeY * 0.78f;
	float Gap = 15.0f;

	FVector2D RestartPos(CenterX - BtnSize.X * 0.5f, BtnY);
	DrawButton(TEXT("PLAY AGAIN"), RestartPos, BtnSize, FColor::White, FLinearColor(0.1f, 0.5f, 0.2f, 0.9f));
	AddButton(TEXT("PLAY AGAIN"), RestartPos, BtnSize, FName(TEXT("ScoreRestart")));

	FVector2D MenuPos(CenterX - BtnSize.X * 0.5f, BtnY + BtnSize.Y + Gap);
	DrawButton(TEXT("MAIN MENU"), MenuPos, BtnSize, FColor::White, FLinearColor(0.3f, 0.3f, 0.4f, 0.9f));
	AddButton(TEXT("MAIN MENU"), MenuPos, BtnSize, FName(TEXT("ScoreMenu")));
}

// --- Gameplay HUD ---

void APowderHUD::DrawGameplayHUD(float DeltaTime)
{
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

	// --- Pause button (top-left) ---
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.4f), 10.0f, 10.0f, 60.0f, 60.0f);
	DrawText(TEXT("||"), FColor::White, 24.0f, 18.0f, GEngine->GetLargeFont(), 1.0f);

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

		float TrickW, TrickH;
		GetTextSize(TrickNotificationText, TrickW, TrickH, GEngine->GetLargeFont(), 1.0f);

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
