#include "UI/PowderHUD.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Core/PowderTuningProfile.h"
#include "Scoring/ScoreSubsystem.h"
#include "Core/PowderGameMode.h"
#include "Core/PowderGameInstance.h"
#include "Terrain/TerrainManager.h"
#include "Core/PowderEnvironmentSetup.h"
#include "Effects/PowderWeatherManager.h"
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
	// Small touch padding around the visual button for finger imprecision
	float Pad = S(UITouchPadding);
	Btn.HitPos = FVector2D(Pos.X - Pad, Pos.Y - Pad);
	Btn.HitSize = FVector2D(Size.X + 2.0f * Pad, Size.Y + 2.0f * Pad);
	Btn.Action = Action;
	ActiveButtons.Add(Btn);
}

FName APowderHUD::TestButtonHit(float X, float Y) const
{
	for (const FScreenButton& Btn : ActiveButtons)
	{
		if (X >= Btn.HitPos.X && X <= Btn.HitPos.X + Btn.HitSize.X &&
			Y >= Btn.HitPos.Y && Y <= Btn.HitPos.Y + Btn.HitSize.Y)
		{
			return Btn.Action;
		}
	}
	return NAME_None;
}

FVector2D APowderHUD::ViewportToCanvas(float X, float Y) const
{
	if (CachedCanvasW <= 0.0f)
	{
		return FVector2D(X, Y);
	}
	int32 VPW, VPH;
	GetOwningPlayerController()->GetViewportSize(VPW, VPH);
	if (VPW <= 0 || VPH <= 0)
	{
		return FVector2D(X, Y);
	}
	// Canvas is centered within the viewport; safe area (notch/home indicator)
	// creates the size difference. Offset, don't scale.
	float XOffset = ((float)VPW - CachedCanvasW) * 0.5f;
	float YOffset = ((float)VPH - CachedCanvasH) * 0.5f;
	return FVector2D(X - XOffset, Y - YOffset);
}

void APowderHUD::DrawButton(const FString& Label, FVector2D Pos, FVector2D Size, FColor TextColor, FLinearColor BgColor)
{
	DrawRect(BgColor, Pos.X, Pos.Y, Size.X, Size.Y);

	float TextW, TextH;
	GetTextSize(Label, TextW, TextH, GEngine->GetLargeFont(), UIScale);
	DrawText(Label, TextColor,
		Pos.X + (Size.X - TextW) * 0.5f,
		Pos.Y + (Size.Y - TextH) * 0.5f,
		GEngine->GetLargeFont(), UIScale);
}

bool APowderHUD::OnMenuTap(float X, float Y)
{
	FVector2D CanvasPos = ViewportToCanvas(X, Y);
	FName Action = TestButtonHit(CanvasPos.X, CanvasPos.Y);
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
		bShowingDevMenu = false;
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
		bShowingDevMenu = false;
		GM->ResumeRun();
	}
	else if (Action == FName(TEXT("Restart")))
	{
		bShowingDevMenu = false;
		GM->RestartRun();
	}
	else if (Action == FName(TEXT("QuitToMenu")))
	{
		bShowingDevMenu = false;
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
	else if (Action == FName(TEXT("DevMenu")))
	{
		bShowingDevMenu = true;
		DevMenuScrollOffset = 0;
		BuildDevParamList();
	}
	else if (Action == FName(TEXT("DevBack")))
	{
		bShowingDevMenu = false;
	}
	else if (Action == FName(TEXT("DevUp")))
	{
		DevMenuScrollOffset = FMath::Max(0, DevMenuScrollOffset - 1);
	}
	else if (Action == FName(TEXT("DevDown")))
	{
		DevMenuScrollOffset = FMath::Min(DevMenuScrollOffset + 1, FMath::Max(0, DevParams.Num() - DevMenuTotalVisible));
	}
	else if (Action == FName(TEXT("DevReset")))
	{
		APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn());
		if (Char)
		{
			if (Char->FeelPresetLadder.Num() > 0)
			{
				Char->ApplyFeelPresetByIndex(0);
			}
			else if (Char->DefaultTuningProfile)
			{
				Char->ApplyTuningProfile(Char->DefaultTuningProfile);
			}
			BuildDevParamList();
		}
	}
	else if (Action == FName(TEXT("DevPresetPrev")))
	{
		if (APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn()))
		{
			Char->StepFeelPreset(-1);
		}
	}
	else if (Action == FName(TEXT("DevPresetNext")))
	{
		if (APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn()))
		{
			Char->StepFeelPreset(1);
		}
	}
	else if (Action == FName(TEXT("DevPresetAB")))
	{
		if (APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn()))
		{
			Char->ToggleLastFeelPreset();
		}
	}
	else if (Action == FName(TEXT("DevAdvanced")))
	{
		bAdvancedDevMenu = !bAdvancedDevMenu;
		DevMenuScrollOffset = 0;
		BuildDevParamList();
	}
	else
	{
		// Dev_Inc_N / Dev_Dec_N
		FString ActionStr = Action.ToString();
		if (ActionStr.StartsWith(TEXT("Dev_Inc_")) || ActionStr.StartsWith(TEXT("Dev_Dec_")))
		{
			bool bInc = ActionStr.StartsWith(TEXT("Dev_Inc_"));
			FString IndexStr = ActionStr.RightChop(8);
			int32 Idx = FCString::Atoi(*IndexStr);
			if (DevParams.IsValidIndex(Idx) && DevParams[Idx].ValuePtr)
			{
				float Delta = bInc ? DevParams[Idx].Step : -DevParams[Idx].Step;
				*DevParams[Idx].ValuePtr = FMath::Clamp(*DevParams[Idx].ValuePtr + Delta, DevParams[Idx].Min, DevParams[Idx].Max);
			}
		}
	}

	return true;
}

bool APowderHUD::IsPauseAreaHit(float X, float Y) const
{
	FVector2D CanvasPos = ViewportToCanvas(X, Y);
	float PauseSize = S(80.0f);
	return CanvasPos.X <= PauseSize && CanvasPos.Y <= PauseSize;
}

// --- DrawHUD ---

void APowderHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	CachedCanvasW = Canvas->SizeX;
	CachedCanvasH = Canvas->SizeY;

	// DPI scaling: design reference is 1080 (narrow) x 1920 (tall)
	// Orientation-independent: always measure narrow vs 1080, tall vs 1920
	float NarrowDim = FMath::Min(CachedCanvasW, CachedCanvasH);
	float TallDim = FMath::Max(CachedCanvasW, CachedCanvasH);
	UIScale = FMath::Min(NarrowDim / 1080.0f, TallDim / 1920.0f);
	UIScale = FMath::Max(UIScale, 0.5f);

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
		if (bShowingDevMenu)
		{
			DrawDevMenu();
		}
		else if (bShowingStats)
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
		if (bShowingDevMenu)
		{
			DrawDevMenu();
		}
		else
		{
			DrawPauseMenu();
		}
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
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), UIScale);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, TitleY, GEngine->GetLargeFont(), UIScale);

	// Subtitle
	FString Subtitle = TEXT("Arcade Skiing");
	float SubW, SubH;
	GetTextSize(Subtitle, SubW, SubH, GEngine->GetSmallFont(), UIScale);
	DrawText(Subtitle, FColor(180, 200, 230), CenterX - SubW * 0.5f, TitleY + TitleH + S(8.0f), GEngine->GetSmallFont(), UIScale);

	// Play button
	FVector2D BtnSize(S(220.0f), S(60.0f));
	float BtnY = Canvas->SizeY * 0.45f;
	FVector2D PlayPos(CenterX - BtnSize.X * 0.5f, BtnY);
	DrawButton(TEXT("PLAY"), PlayPos, BtnSize, FColor::White, FLinearColor(0.1f, 0.5f, 0.2f, 0.9f));
	AddButton(TEXT("PLAY"), PlayPos, BtnSize, FName(TEXT("Play")));

	// Stats button
	float StatsY = BtnY + BtnSize.Y + S(20.0f);
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
			GetTextSize(HighScoreText, HSW, HSH, GEngine->GetSmallFont(), UIScale);
			DrawText(HighScoreText, FColor(255, 220, 80), CenterX - HSW * 0.5f, StatsY + BtnSize.Y + S(30.0f), GEngine->GetSmallFont(), UIScale);
		}
	}

	// DEV button (bottom-right)
	FVector2D DevSize(S(80.0f), S(40.0f));
	FVector2D DevPos(Canvas->SizeX - DevSize.X - S(15.0f), Canvas->SizeY - DevSize.Y - S(15.0f));
	DrawButton(TEXT("DEV"), DevPos, DevSize, FColor(200, 200, 200), FLinearColor(0.3f, 0.3f, 0.3f, 0.7f));
	AddButton(TEXT("DEV"), DevPos, DevSize, FName(TEXT("DevMenu")));
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
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), UIScale);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, TitleY, GEngine->GetLargeFont(), UIScale);

	FVector2D BtnSize(S(220.0f), S(55.0f));
	float BtnX = CenterX - BtnSize.X * 0.5f;
	float BtnY = Canvas->SizeY * 0.4f;
	float Gap = S(15.0f);

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

	// DEV button (bottom-right)
	FVector2D DevSize(S(80.0f), S(40.0f));
	FVector2D DevPos(Canvas->SizeX - DevSize.X - S(15.0f), Canvas->SizeY - DevSize.Y - S(15.0f));
	DrawButton(TEXT("DEV"), DevPos, DevSize, FColor(200, 200, 200), FLinearColor(0.3f, 0.3f, 0.3f, 0.7f));
	AddButton(TEXT("DEV"), DevPos, DevSize, FName(TEXT("DevMenu")));
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
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), UIScale);
	DrawText(Title, FColor::White, CenterX - TitleW * 0.5f, Y, GEngine->GetLargeFont(), UIScale);
	Y += TitleH + S(20.0f);

	UPowderGameInstance* GI = Cast<UPowderGameInstance>(GetOwningPlayerController()->GetGameInstance());
	if (GI)
	{
		const FLifetimeStats& Stats = GI->GetLifetimeStats();
		float LeftX = Canvas->SizeX * 0.1f;
		float LineH = S(22.0f);

		auto DrawStat = [&](const FString& Label, const FString& Value)
		{
			DrawText(Label, FColor(180, 200, 220), LeftX, Y, GEngine->GetSmallFont(), UIScale);
			float ValW, ValH;
			GetTextSize(Value, ValW, ValH, GEngine->GetSmallFont(), UIScale);
			DrawText(Value, FColor(255, 220, 80), Canvas->SizeX * 0.9f - ValW, Y, GEngine->GetSmallFont(), UIScale);
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

		Y += S(10.0f);
		DrawText(TEXT("--- Lifetime Totals ---"), FColor(140, 160, 180), LeftX, Y, GEngine->GetSmallFont(), UIScale);
		Y += LineH;

		DrawStat(TEXT("Total Coins Earned"), FString::Printf(TEXT("%d"), Stats.TotalCoinsEarned));
		DrawStat(TEXT("Total Tricks Landed"), FString::Printf(TEXT("%d"), Stats.TotalTricksLanded));
		DrawStat(TEXT("Total Near Misses"), FString::Printf(TEXT("%d"), Stats.TotalNearMisses));
		DrawStat(TEXT("Total Gates Passed"), FString::Printf(TEXT("%d"), Stats.TotalGatesPassed));
		DrawStat(TEXT("Total Distance"), FString::Printf(TEXT("%.0f m"), Stats.TotalDistanceTraveled / 100.0f));
	}

	// Back button
	FVector2D BtnSize(S(180.0f), S(50.0f));
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
	GetTextSize(FinishedText, TextW, TextH, GEngine->GetLargeFont(), UIScale);
	DrawText(FinishedText, FColor::White, CenterX - TextW * 0.5f, Y, GEngine->GetLargeFont(), UIScale);
	Y += TextH + S(15.0f);

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
		GetTextSize(ScoreText, ScoreW, ScoreH, GEngine->GetLargeFont(), UIScale);
		DrawText(ScoreText, FColor(255, 220, 80), CenterX - ScoreW * 0.5f, Y, GEngine->GetLargeFont(), UIScale);
		Y += ScoreH + S(5.0f);

		// New best indicator
		if (GI && RunStats.TotalScore >= GI->GetHighScore() && RunStats.TotalScore > 0)
		{
			FString BestText = TEXT("NEW BEST!");
			float BestW, BestH;
			GetTextSize(BestText, BestW, BestH, GEngine->GetLargeFont(), UIScale);
			DrawText(BestText, FColor(80, 255, 80), CenterX - BestW * 0.5f, Y, GEngine->GetLargeFont(), UIScale);
			Y += BestH + S(5.0f);
		}

		Y += S(10.0f);
		float LeftX = Canvas->SizeX * 0.2f;
		float LineH = S(20.0f);

		auto DrawRunStat = [&](const FString& Label, const FString& Value)
		{
			DrawText(Label, FColor(200, 200, 200), LeftX, Y, GEngine->GetSmallFont(), UIScale);
			float ValW, ValH;
			GetTextSize(Value, ValW, ValH, GEngine->GetSmallFont(), UIScale);
			DrawText(Value, FColor::White, Canvas->SizeX * 0.8f - ValW, Y, GEngine->GetSmallFont(), UIScale);
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
	FVector2D BtnSize(S(200.0f), S(50.0f));
	float BtnY = Canvas->SizeY * 0.78f;
	float Gap = S(15.0f);

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
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.4f), S(10.0f), S(10.0f), S(60.0f), S(60.0f));
	DrawText(TEXT("||"), FColor::White, S(24.0f), S(18.0f), GEngine->GetLargeFont(), UIScale);

	// --- Score display (top-center) ---
	UScoreSubsystem* ScoreSys = GetOwningPlayerController()
		? GetOwningPlayerController()->GetGameInstance()->GetSubsystem<UScoreSubsystem>()
		: nullptr;

	if (ScoreSys)
	{
		float CenterX = Canvas->SizeX * 0.5f;
		float ScoreY = S(20.0f);

		// Score
		FString ScoreText = FString::Printf(TEXT("%d"), ScoreSys->GetCurrentScore());
		float ScoreW, ScoreH;
		GetTextSize(ScoreText, ScoreW, ScoreH, GEngine->GetLargeFont(), UIScale);
		DrawText(ScoreText, FColor::White, CenterX - ScoreW * 0.5f, ScoreY, GEngine->GetLargeFont(), UIScale);

		// Multiplier (only when > 1.0x)
		float Multiplier = ScoreSys->GetMultiplier();
		if (Multiplier > 1.0f)
		{
			FString MultText = FString::Printf(TEXT("x%.1f"), Multiplier);
			float MultW, MultH;
			GetTextSize(MultText, MultW, MultH, GEngine->GetLargeFont(), UIScale);
			DrawText(MultText, FColor(255, 180, 0), CenterX - MultW * 0.5f, ScoreY + ScoreH + S(2.0f), GEngine->GetLargeFont(), UIScale);
		}

		// Combo timer bar (below score)
		float ComboNorm = ScoreSys->GetComboTimerNormalized();
		if (ComboNorm > 0.0f)
		{
			float ComboBarWidth = S(150.0f);
			float ComboBarHeight = S(4.0f);
			float ComboBarX = CenterX - ComboBarWidth * 0.5f;
			float ComboBarY = ScoreY + ScoreH + S(24.0f);

			DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.6f), ComboBarX, ComboBarY, ComboBarWidth, ComboBarHeight);
			DrawRect(FLinearColor(1.0f, 0.7f, 0.0f, 1.0f), ComboBarX, ComboBarY, ComboBarWidth * ComboNorm, ComboBarHeight);
		}
	}

	// --- Powerup indicator (top-right area) ---
	if (bPowerupActive)
	{
		float IndicatorX = Canvas->SizeX - S(200.0f);
		float IndicatorY = S(20.0f);
		float BarWidth = S(160.0f);
		float BarHeight = S(8.0f);

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

		DrawText(LabelText, LabelColor, IndicatorX, IndicatorY, GEngine->GetSmallFont(), UIScale);

		float TimerNorm = (PowerupDuration > 0.0f) ? FMath::Clamp(PowerupTimeRemaining / PowerupDuration, 0.0f, 1.0f) : 0.0f;
		float TimerBarY = IndicatorY + S(20.0f);
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
		GetTextSize(PowerupFlashText, FlashW, FlashH, GEngine->GetLargeFont(), UIScale);
		DrawText(PowerupFlashText, FlashColor,
			CenterX - FlashW * 0.5f, FlashY - FlashH * 0.5f,
			GEngine->GetLargeFont(), UIScale);
	}

	// --- Trick notification (center screen, brief flash) ---
	if (TrickNotificationTimer > 0.0f)
	{
		float CenterX = Canvas->SizeX * 0.5f;
		float CenterY = Canvas->SizeY * 0.35f;

		float Alpha = FMath::Clamp(TrickNotificationTimer / 0.3f, 0.0f, 1.0f);

		float TrickW, TrickH;
		GetTextSize(TrickNotificationText, TrickW, TrickH, GEngine->GetLargeFont(), UIScale);

		FColor TrickColor = TrickNotificationText.Contains(TEXT("WIPEOUT"))
			? FColor(255, 80, 80, FMath::RoundToInt32(Alpha * 255))
			: FColor(255, 255, 80, FMath::RoundToInt32(Alpha * 255));

		DrawText(TrickNotificationText, TrickColor,
			CenterX - TrickW * 0.5f, CenterY - TrickH * 0.5f,
			GEngine->GetLargeFont(), UIScale);
	}

	// --- Bottom-left debug panel ---
	float BarWidth = S(250.0f);
	float BarHeight = S(16.0f);
	float Padding = S(20.0f);
	float TextH = S(20.0f);
	float Gap = S(6.0f);

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
	DrawText(SpeedText, FColor::White, Padding, Y, GEngine->GetSmallFont(), UIScale);
	Y += TextH + Gap;

	// Speed bar
	DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f), Padding, Y, BarWidth, BarHeight);
	FLinearColor SpeedColor = FLinearColor::LerpUsingHSV(
		FLinearColor(0.2f, 0.8f, 0.2f), FLinearColor(1.0f, 0.2f, 0.1f), SpeedNorm);
	DrawRect(SpeedColor, Padding, Y, BarWidth * SpeedNorm, BarHeight);
	Y += BarHeight + Gap;

	// Boost text
	FString BoostText = FString::Printf(TEXT("Boost: %.0f%%"), Boost * 100.0f);
	DrawText(BoostText, FColor(100, 180, 255), Padding, Y, GEngine->GetSmallFont(), UIScale);
	Y += TextH + Gap;

	// Boost bar
	DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f), Padding, Y, BarWidth, BarHeight);
	DrawRect(FLinearColor(0.2f, 0.6f, 1.0f), Padding, Y, BarWidth * Boost, BarHeight);
	Y += BarHeight;

	// Airborne indicator
	if (bAirborne)
	{
		Y += Gap;
		DrawText(TEXT("AIRBORNE"), FColor(80, 255, 80), Padding, Y, GEngine->GetSmallFont(), UIScale);
	}
}

// --- Dev Tuning Menu ---

void APowderHUD::BuildDevParamList()
{
	DevParams.Empty();

	APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn());
	if (!Char)
	{
		return;
	}

	UPowderMovementComponent* MC = Char->GetPowderMovement();
	if (!MC)
	{
		return;
	}

	// Helper to add a param with a category header marker (Label starting with "--")
	auto Add = [&](const FString& Label, float* Ptr, float Step, float Min, float Max)
	{
		FDevTuningParam P;
		P.Label = Label;
		P.ValuePtr = Ptr;
		P.Step = Step;
		P.Min = Min;
		P.Max = Max;
		DevParams.Add(P);
	};

	// Category header (ValuePtr = nullptr signals a header row)
	auto AddHeader = [&](const FString& Label)
	{
		FDevTuningParam P;
		P.Label = Label;
		P.ValuePtr = nullptr;
		P.Step = 0.0f;
		P.Min = 0.0f;
		P.Max = 0.0f;
		DevParams.Add(P);
	};

	if (!bAdvancedDevMenu)
	{
		return;
	}

	// -- Movement params --
	AddHeader(TEXT("-- Movement --"));
	Add(TEXT("GravityAccel"),      &MC->GravityAcceleration,  50.0f,   100.0f,  5000.0f);
	Add(TEXT("SlopeAngle"),        &MC->SlopeAngle,           1.0f,    0.0f,    60.0f);
	Add(TEXT("GravSlopeScale"),    &MC->GravityAlongSlopeScale,0.02f,  0.05f,   1.5f);
	Add(TEXT("MaxAccelSlope"),     &MC->MaxAccelerationSlopeAngle,1.0f,5.0f,    60.0f);
	Add(TEXT("MaxSpeed"),          &MC->MaxSpeed,             50.0f,   500.0f,  10000.0f);
	Add(TEXT("BaseFriction"),      &MC->BaseFriction,         0.005f,  0.0f,    0.5f);
	Add(TEXT("CarveSpeedBleed"),   &MC->CarveSpeedBleed,      0.05f,   0.0f,    5.0f);
	Add(TEXT("CarveBleedSmooth"),  &MC->CarveBleedSmoothing,  0.05f,   0.0f,    10.0f);
	Add(TEXT("CarveRate"),         &MC->CarveRate,            0.05f,   0.0f,    5.0f);
	Add(TEXT("CarveReturnRate"),   &MC->CarveReturnRate,      0.05f,   0.0f,    5.0f);
	Add(TEXT("MaxCarveAngle"),     &MC->MaxCarveAngle,        1.0f,    10.0f,   180.0f);
	Add(TEXT("YawRate"),           &MC->YawRate,              1.0f,    10.0f,   360.0f);
	Add(TEXT("CarveLateralSpd"),   &MC->CarveLateralSpeed,    50.0f,   0.0f,    5000.0f);
	Add(TEXT("BoostFillRate"),     &MC->BoostFillRate,        0.05f,   0.0f,    5.0f);
	Add(TEXT("BoostBurstSpeed"),   &MC->BoostBurstSpeed,      50.0f,   0.0f,    5000.0f);
	Add(TEXT("BoostDuration"),     &MC->BoostDuration,        0.05f,   0.0f,    5.0f);
	Add(TEXT("OllieForce"),        &MC->OllieForce,           50.0f,   0.0f,    3000.0f);
	Add(TEXT("OllieCooldown"),     &MC->OllieCooldown,        0.05f,   0.0f,    5.0f);
	Add(TEXT("TerrainContactOff"), &MC->TerrainContactOffset, 1.0f,   -30.0f,   30.0f);
	Add(TEXT("CarveInputSmooth"),  &MC->CarveInputSmoothing,  0.05f,   0.0f,    20.0f);
	Add(TEXT("CarveRampTime"),     &MC->CarveRampTime,        0.05f,   0.0f,    3.0f);
	Add(TEXT("CarveRampMinInt"),   &MC->CarveRampMinIntensity,0.05f,   0.0f,    1.0f);
	Add(TEXT("CarveRampEaseExp"), &MC->CarveRampEaseExponent, 0.05f,   0.5f,    5.0f);
	Add(TEXT("SpeedTurnLimit"),  &MC->SpeedTurnLimitFactor,  0.05f,   0.0f,    1.0f);
	Add(TEXT("MinTurnAtMaxSpd"), &MC->MinTurnAngleAtMaxSpeed,1.0f,    5.0f,    90.0f);
	Add(TEXT("YawSmoothing"),    &MC->YawSmoothing,          0.01f,   0.0f,    0.5f);
	Add(TEXT("MinGroundZ"),      &MC->MinGroundNormalZ,      0.01f,   0.01f,   1.0f);
	Add(TEXT("NormFilterSpd"),   &MC->GroundNormalFilterSpeed,0.1f,   0.0f,    50.0f);
	Add(TEXT("DownhillAlign"),   &MC->DownhillAlignRate,     1.0f,    0.0f,    360.0f);
	Add(TEXT("TurnRateLimit"),   &MC->TurnRateLimitDegPerSec,1.0f,    10.0f,   720.0f);
	Add(TEXT("CarveBleedExp"),   &MC->CarveBleedExponent,    0.05f,   0.1f,    4.0f);

	// -- Ski Feel params --
	AddHeader(TEXT("-- Ski Feel --"));
	Add(TEXT("SpdTurnRateMin"),  &MC->SpeedTurnRateMin,             0.05f,   0.1f,    1.0f);
	Add(TEXT("SpdTurnRateExp"),  &MC->SpeedTurnRateExponent,        0.05f,   0.1f,    4.0f);
	Add(TEXT("EdgeEngageRate"),  &MC->EdgeEngageRate,               0.5f,    0.5f,    20.0f);
	Add(TEXT("EdgeDisengRate"),  &MC->EdgeDisengageRate,            0.5f,    0.5f,    20.0f);
	Add(TEXT("EdgeMinDepth"),    &MC->EdgeMinDepth,                 0.05f,   0.0f,    1.0f);
	Add(TEXT("HeadTraverse"),    &MC->HeadingTraverseFactor,        0.05f,  -0.5f,    1.0f);
	Add(TEXT("HeadUphill"),      &MC->HeadingUphillFactor,          0.05f,  -1.0f,    0.5f);
	Add(TEXT("HeadFriction"),    &MC->HeadingFrictionScale,         0.05f,   0.0f,    1.0f);
	Add(TEXT("TurnCommitTime"),  &MC->TurnCommitTime,              0.05f,   0.0f,    1.0f);
	Add(TEXT("TurnCommitDecay"), &MC->TurnCommitDecay,             0.05f,   0.0f,    1.0f);
	Add(TEXT("LandSpdPenalty"),  &MC->LandingSpeedPenaltyMax,      0.05f,   0.0f,    1.0f);
	Add(TEXT("LandCtrlDur"),     &MC->LandingControlPenaltyDuration,0.05f,  0.0f,    2.0f);
	Add(TEXT("LandCtrlFactor"),  &MC->LandingControlPenaltyFactor, 0.05f,   0.0f,    1.0f);
	Add(TEXT("LandQualThresh"),  &MC->LandingQualityThreshold,     0.05f,   0.0f,    1.0f);

	// -- Terrain & Air params --
	AddHeader(TEXT("-- Terrain & Air --"));
	Add(TEXT("TerrSnapThresh"),  &MC->TerrainSnapThreshold,       0.5f,    0.5f,    20.0f);
	Add(TEXT("LandBlendDur"),    &MC->LandingBlendDuration,       0.01f,   0.0f,    0.5f);
	Add(TEXT("AirDrag"),         &MC->AirDragCoefficient,         0.05f,   0.0f,    2.0f);
	Add(TEXT("AirTerminalVel"),  &MC->AirTerminalVelocity,        50.0f,   200.0f,  5000.0f);

	// -- Edge Feel params --
	AddHeader(TEXT("-- Edge Feel --"));
	Add(TEXT("EdgeTransTime"),   &MC->EdgeTransitionTime,         0.01f,   0.0f,    0.5f);
	Add(TEXT("EdgeTransGrip"),   &MC->EdgeTransitionGrip,         0.05f,   0.0f,    1.0f);
	Add(TEXT("PressureBuild"),   &MC->CarvePressureBuildRate,     0.1f,    0.1f,    5.0f);
	Add(TEXT("PressureDecay"),   &MC->CarvePressureDecayRate,     0.1f,    0.5f,    10.0f);
	Add(TEXT("PressureTurnBns"), &MC->CarvePressureTurnBonus,     0.05f,   0.0f,    0.5f);
	Add(TEXT("PressureBleedBns"),&MC->CarvePressureBleedBonus,    0.05f,   0.0f,    0.5f);

	// -- Camera params --
	AddHeader(TEXT("-- Camera --"));
	Add(TEXT("BaseArmLen"),        &Char->BaseArmLength,       50.0f,   200.0f,  5000.0f);
	Add(TEXT("MaxArmLen"),         &Char->MaxArmLength,        50.0f,   200.0f,  5000.0f);
	Add(TEXT("ArmLenInterp"),      &Char->ArmLengthInterpSpeed,0.05f,  0.1f,    20.0f);
	Add(TEXT("BasePitch"),         &Char->BasePitch,           1.0f,    -89.0f,  0.0f);
	Add(TEXT("SpeedPitch"),        &Char->SpeedPitch,          1.0f,    -89.0f,  0.0f);
	Add(TEXT("BaseYawOffset"),     &Char->BaseYawOffset,       1.0f,    -90.0f,  90.0f);
	Add(TEXT("CamHeadingFollow"),  &Char->CameraHeadingFollow, 0.05f,   0.0f,    1.0f);
	Add(TEXT("CamYawInterp"),      &Char->CameraYawInterpSpeed,0.05f,  0.01f,   10.0f);
	Add(TEXT("CamTurnLead"),       &Char->CameraTurnLeadWeight,0.05f,  0.0f,    1.0f);
	Add(TEXT("CamLookAhead"),      &Char->CameraLookAheadWeight,0.05f, 0.0f,    1.0f);
	Add(TEXT("CamLookMaxYaw"),     &Char->CameraLookAheadMaxYaw,1.0f,  0.0f,    90.0f);
	Add(TEXT("BaseFOV"),           &Char->BaseFOV,             1.0f,    30.0f,   120.0f);
	Add(TEXT("MaxFOV"),            &Char->MaxFOV,              1.0f,    30.0f,   120.0f);
	Add(TEXT("FOVInterp"),         &Char->FOVInterpSpeed,      0.05f,   0.1f,    20.0f);

	// -- Surface params --
	AddHeader(TEXT("-- Surface --"));
	Add(TEXT("Friction"),          &MC->CurrentSurface.Friction,          0.1f,    0.0f,    5.0f);
	Add(TEXT("CarveGrip"),         &MC->CurrentSurface.CarveGrip,         0.1f,    0.0f,    3.0f);
	Add(TEXT("SpeedMultiplier"),   &MC->CurrentSurface.SpeedMultiplier,   0.05f,   0.5f,    2.0f);
	Add(TEXT("SnowSprayAmount"),   &MC->CurrentSurface.SnowSprayAmount,   0.1f,    0.0f,    3.0f);
	Add(TEXT("SurfBlendTime"),     &MC->CurrentSurface.BlendTime,         0.1f,    0.0f,    3.0f);

	// -- Terrain params --
	APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode());
	ATerrainManager* TM = GM ? GM->GetTerrainManager() : nullptr;
	if (TM)
	{
		AddHeader(TEXT("-- Terrain --"));
		Add(TEXT("StartOffset"),     &TM->StartDistanceOffset,            50.0f,    0.0f,     3000.0f);
		Add(TEXT("RespawnBack"),     &TM->RespawnBacktrackDistance,       100.0f,   200.0f,   10000.0f);
	}

	// -- Weather params --
	UPowderWeatherManager* WM = nullptr;
	if (GM)
	{
		if (APowderEnvironmentSetup* Env = GM->GetEnvironmentSetup())
		{
			WM = Env->GetWeatherManager();
		}
	}
	if (WM)
	{
		AddHeader(TEXT("-- Weather --"));
		Add(TEXT("SunIntensity"),    &WM->CurrentWeather.SunIntensity,    0.5f,    0.0f,    10.0f);
		Add(TEXT("FogDensity"),      &WM->CurrentWeather.FogDensity,      0.001f,  0.0f,    0.05f);
		Add(TEXT("FogStartDist"),    &WM->CurrentWeather.FogStartDistance, 100.0f,  0.0f,    5000.0f);
		Add(TEXT("SnowfallRate"),    &WM->CurrentWeather.SnowfallRate,    0.5f,    0.0f,    10.0f);
		Add(TEXT("WindStrength"),    &WM->CurrentWeather.WindStrength,    0.5f,    0.0f,    10.0f);
		Add(TEXT("SkyLightIntens"), &WM->CurrentWeather.SkyLightIntensity, 0.1f,  0.0f,    5.0f);
	}

	// -- UI params --
	APowderHUD* HUD = this;
	AddHeader(TEXT("-- UI --"));
	Add(TEXT("UITouchPadding"),    &HUD->UITouchPadding,       2.0f,    4.0f,    30.0f);
}

void APowderHUD::DrawDevMenu()
{
	// Dark background
	DrawRect(FLinearColor(0.05f, 0.05f, 0.1f, 0.95f), 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);

	UFont* ParamFont = GEngine->GetMediumFont();

	const float Margin = S(14.0f);
	const float RowH = S(52.0f);
	const float BtnW = S(60.0f);
	const float BtnH = S(44.0f);
	const float BottomBarH = S(56.0f);
	const float BottomPad = S(10.0f);
	const float FontScale = UIScale * 1.2f;

	// --- Title bar ---
	float TitleY = S(10.0f);
	FString Title = TEXT("DEV TUNING");
	float TitleW, TitleH;
	GetTextSize(Title, TitleW, TitleH, GEngine->GetLargeFont(), FontScale);
	DrawText(Title, FColor(255, 200, 80), Canvas->SizeX * 0.5f - TitleW * 0.5f, TitleY, GEngine->GetLargeFont(), FontScale);

	// [X] close button (top-right)
	FVector2D CloseSize(BtnW, BtnH);
	FVector2D ClosePos(Canvas->SizeX - Margin - BtnW, TitleY);
	DrawButton(TEXT("X"), ClosePos, CloseSize, FColor::White, FLinearColor(0.5f, 0.15f, 0.15f, 0.9f));
	AddButton(TEXT("X"), ClosePos, CloseSize, FName(TEXT("DevBack")));

	float TopY = TitleY + TitleH + S(8.0f);

	// --- Feel Lab panel ---
	APowderCharacter* Char = Cast<APowderCharacter>(GetOwningPawn());
	UPowderMovementComponent* MoveComp = Char ? Char->GetPowderMovement() : nullptr;

	const float FeelPanelH = S(136.0f);
	const float PanelW = Canvas->SizeX - 2.0f * Margin;
	DrawRect(FLinearColor(0.08f, 0.09f, 0.14f, 0.96f), Margin, TopY, PanelW, FeelPanelH);

	float FeelY = TopY + S(8.0f);
	DrawText(TEXT("FEEL LAB"), FColor(255, 220, 140), Margin + S(10.0f), FeelY, ParamFont, FontScale);
	FeelY += S(30.0f);

	const FString PresetLabel = Char ? Char->GetActiveFeelPresetName() : FString(TEXT("No Character"));
	DrawText(FString::Printf(TEXT("Preset: %s"), *PresetLabel), FColor(220, 220, 220), Margin + S(10.0f), FeelY, ParamFont, FontScale);
	FeelY += S(24.0f);

	if (MoveComp)
	{
		const FString Metrics = FString::Printf(
			TEXT("Speed %.0f | Carve %.1f | TurnRate %.1f | NormalStability %.2f"),
			MoveComp->GetCurrentSpeed(),
			MoveComp->GetCarveAngle(),
			MoveComp->GetTurnRateDegPerSec(),
			MoveComp->GetGroundNormalStability());
		DrawText(Metrics, FColor(170, 205, 255), Margin + S(10.0f), FeelY, GEngine->GetSmallFont(), UIScale);
	}
	else
	{
		DrawText(TEXT("Movement component unavailable"), FColor(200, 120, 120), Margin + S(10.0f), FeelY, GEngine->GetSmallFont(), UIScale);
	}

	const float ControlY = TopY + FeelPanelH - S(50.0f);
	const float PresetBtnW = S(90.0f);
	const float PresetBtnH = S(40.0f);
	const float ControlGap = S(10.0f);
	float ControlX = Margin + S(10.0f);

	FVector2D PrevPos(ControlX, ControlY);
	DrawButton(TEXT("PREV"), PrevPos, FVector2D(PresetBtnW, PresetBtnH), FColor::White, FLinearColor(0.2f, 0.3f, 0.5f, 0.9f));
	AddButton(TEXT("PREV"), PrevPos, FVector2D(PresetBtnW, PresetBtnH), FName(TEXT("DevPresetPrev")));
	ControlX += PresetBtnW + ControlGap;

	FVector2D ABPos(ControlX, ControlY);
	DrawButton(TEXT("A/B"), ABPos, FVector2D(PresetBtnW, PresetBtnH), FColor::White, FLinearColor(0.35f, 0.25f, 0.55f, 0.9f));
	AddButton(TEXT("A/B"), ABPos, FVector2D(PresetBtnW, PresetBtnH), FName(TEXT("DevPresetAB")));
	ControlX += PresetBtnW + ControlGap;

	FVector2D NextPos(ControlX, ControlY);
	DrawButton(TEXT("NEXT"), NextPos, FVector2D(PresetBtnW, PresetBtnH), FColor::White, FLinearColor(0.2f, 0.3f, 0.5f, 0.9f));
	AddButton(TEXT("NEXT"), NextPos, FVector2D(PresetBtnW, PresetBtnH), FName(TEXT("DevPresetNext")));

	const FString AdvLabel = bAdvancedDevMenu ? TEXT("ADV ON") : TEXT("ADV OFF");
	const float AdvBtnW = S(110.0f);
	FVector2D AdvPos(Canvas->SizeX - Margin - AdvBtnW - S(10.0f), ControlY);
	DrawButton(AdvLabel, AdvPos, FVector2D(AdvBtnW, PresetBtnH), FColor::White, FLinearColor(0.35f, 0.35f, 0.35f, 0.9f));
	AddButton(AdvLabel, AdvPos, FVector2D(AdvBtnW, PresetBtnH), FName(TEXT("DevAdvanced")));

	TopY += FeelPanelH + S(10.0f);

	// --- Single-column layout (advanced mode only) ---
	float ColWidth = Canvas->SizeX - 2.0f * Margin;
	float AvailableH = Canvas->SizeY - TopY - BottomBarH - BottomPad;
	int32 RowsVisible = FMath::Max(1, FMath::FloorToInt32(AvailableH / RowH));
	DevMenuTotalVisible = RowsVisible;

	bool bNeedScroll = bAdvancedDevMenu && DevParams.Num() > DevMenuTotalVisible;
	int32 StartIdx = DevMenuScrollOffset;
	int32 EndIdx = FMath::Min(StartIdx + DevMenuTotalVisible, DevParams.Num());

	if (!bAdvancedDevMenu)
	{
		DrawText(TEXT("Advanced sliders hidden. Use Feel Lab preset controls."), FColor(170, 170, 170), Margin, TopY + S(6.0f), ParamFont, FontScale);
	}
	else
	{
		// --- Draw param rows ---
		for (int32 i = StartIdx; i < EndIdx; i++)
		{
			int32 Row = i - StartIdx;
			float RowY = TopY + Row * RowH;

			const FDevTuningParam& P = DevParams[i];

			if (!P.ValuePtr)
			{
				// Header row — colored text, no buttons
				DrawText(P.Label, FColor(180, 220, 255), Margin, RowY + S(10.0f), ParamFont, FontScale);
				continue;
			}

			// [-] button
			FVector2D DecPos(Margin, RowY + S(4.0f));
			FVector2D DecSize(BtnW, BtnH);
			DrawButton(TEXT("-"), DecPos, DecSize, FColor::White, FLinearColor(0.5f, 0.2f, 0.2f, 0.9f));
			AddButton(TEXT("-"), DecPos, DecSize, FName(*FString::Printf(TEXT("Dev_Dec_%d"), i)));

			// Label
			float LabelX = Margin + BtnW + S(8.0f);
			DrawText(P.Label, FColor(200, 200, 200), LabelX, RowY + S(12.0f), ParamFont, FontScale);

			// Value
			FString ValStr;
			if (P.Step >= 1.0f)
			{
				ValStr = FString::Printf(TEXT("%.0f"), *P.ValuePtr);
			}
			else if (P.Step >= 0.01f)
			{
				ValStr = FString::Printf(TEXT("%.2f"), *P.ValuePtr);
			}
			else
			{
				ValStr = FString::Printf(TEXT("%.3f"), *P.ValuePtr);
			}

			float ValW, ValH;
			GetTextSize(ValStr, ValW, ValH, ParamFont, FontScale);
			float IncX = Margin + ColWidth - BtnW;
			DrawText(ValStr, FColor::White, IncX - ValW - S(8.0f), RowY + S(12.0f), ParamFont, FontScale);

			// [+] button
			FVector2D IncPos(IncX, RowY + S(4.0f));
			FVector2D IncSize(BtnW, BtnH);
			DrawButton(TEXT("+"), IncPos, IncSize, FColor::White, FLinearColor(0.2f, 0.5f, 0.2f, 0.9f));
			AddButton(TEXT("+"), IncPos, IncSize, FName(*FString::Printf(TEXT("Dev_Inc_%d"), i)));
		}
	}

	// --- Bottom bar ---
	float BottomY = Canvas->SizeY - BottomBarH - BottomPad;
	float NavBtnW = S(90.0f);
	float NavBtnH = S(46.0f);

	// RESET (centered)
	float ResetW = S(110.0f);
	FVector2D ResetPos(Canvas->SizeX * 0.5f - ResetW * 0.5f, BottomY);
	DrawButton(TEXT("RESET"), ResetPos, FVector2D(ResetW, NavBtnH), FColor::White, FLinearColor(0.5f, 0.4f, 0.1f, 0.9f));
	AddButton(TEXT("RESET"), ResetPos, FVector2D(ResetW, NavBtnH), FName(TEXT("DevReset")));

	// UP/DOWN scroll buttons — always show when needed
	if (bNeedScroll)
	{
		float BottomGap = S(10.0f);
		FVector2D UpPos(Canvas->SizeX - Margin - NavBtnW * 2.0f - BottomGap, BottomY);
		DrawButton(TEXT("UP"), UpPos, FVector2D(NavBtnW, NavBtnH), FColor::White, FLinearColor(0.25f, 0.25f, 0.35f, 0.9f));
		AddButton(TEXT("UP"), UpPos, FVector2D(NavBtnW, NavBtnH), FName(TEXT("DevUp")));

		FVector2D DownPos(Canvas->SizeX - Margin - NavBtnW, BottomY);
		DrawButton(TEXT("DOWN"), DownPos, FVector2D(NavBtnW, NavBtnH), FColor::White, FLinearColor(0.25f, 0.25f, 0.35f, 0.9f));
		AddButton(TEXT("DOWN"), DownPos, FVector2D(NavBtnW, NavBtnH), FName(TEXT("DevDown")));

		// Scroll info
		FString ScrollInfo = FString::Printf(TEXT("%d-%d / %d"), StartIdx + 1, EndIdx, DevParams.Num());
		DrawText(ScrollInfo, FColor(140, 140, 140), Margin, BottomY + S(12.0f), ParamFont, FontScale);
	}

}
