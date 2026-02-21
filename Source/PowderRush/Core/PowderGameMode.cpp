// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameMode.h"
#include "Core/PowderEnvironmentSetup.h"
#include "Core/PowderGameInstance.h"
#include "Terrain/TerrainManager.h"
#include "Effects/PowderWeatherManager.h"
#include "Effects/PowderWeatherVolume.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderPlayerController.h"
#include "Scoring/ScoreSubsystem.h"
#include "UI/PowderHUD.h"
#include "Components/SkyLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

APowderGameMode::APowderGameMode()
{
	DefaultPawnClass = APowderCharacter::StaticClass();
	PlayerControllerClass = APowderPlayerController::StaticClass();
	HUDClass = APowderHUD::StaticClass();
	RunState = EPowderRunState::InMenu;
	PrimaryActorTick.bCanEverTick = true;
}

void APowderGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Require pre-placed environment setup.
	TArray<AActor*> ExistingEnv;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APowderEnvironmentSetup::StaticClass(), ExistingEnv);
	if (ExistingEnv.Num() > 0)
	{
		EnvironmentSetup = Cast<APowderEnvironmentSetup>(ExistingEnv[0]);
	}

	// Require pre-placed terrain manager.
	TArray<AActor*> ExistingTerrainManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATerrainManager::StaticClass(), ExistingTerrainManagers);
	if (ExistingTerrainManagers.Num() > 0)
	{
		TerrainManager = Cast<ATerrainManager>(ExistingTerrainManagers[0]);
	}

	if (!EnvironmentSetup)
	{
		UE_LOG(LogTemp, Error, TEXT("PowderGameMode: Missing APowderEnvironmentSetup in level."));
	}

	if (!TerrainManager)
	{
		UE_LOG(LogTemp, Error, TEXT("PowderGameMode: Missing ATerrainManager in level."));
	}

	if (TerrainManager)
	{
		const bool bCourseReady = TerrainManager->InitializeCourse();
		if (!bCourseReady)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("PowderGameMode: Failed to initialize course at BeginPlay. Configure TerrainManager or place APowderCoursePath with spline points."));
		}

		// Position player at slope start and freeze (InMenu state)
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (PlayerPawn && bCourseReady)
		{
			FVector StartPos = TerrainManager->GetSlopeStartPosition();
			FRotator StartRot = TerrainManager->GetStartFacingRotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
		}
	}

	bRequiredActorsReady = AreRequiredActorsReady();
	SetPlayerFrozen(true);
}

void APowderGameMode::StartRun()
{
	SetRunState(EPowderRunState::Starting);

	if (TerrainManager && !TerrainManager->IsCourseInitialized())
	{
		TerrainManager->InitializeCourse();
	}

	bRequiredActorsReady = AreRequiredActorsReady();
	if (!bRequiredActorsReady)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("PowderGameMode: StartRun aborted. Required actors missing or invalid (EnvironmentSetup, TerrainManager, CoursePath spline)."));
		SetPlayerFrozen(true);
		SetRunState(EPowderRunState::InMenu);
		return;
	}

	// Move player to the top of the slope.
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		FVector StartPos = TerrainManager->GetSlopeStartPosition();
		FRotator StartRot = TerrainManager->GetStartFacingRotation();
		PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
	}

	// Force clear day at start, then volume blending takes over during Tick.
	bHasLastAppliedWeather = false;
	ApplyWeatherIfNeeded(UPowderWeatherManager::GetDefaultConfig(EWeatherPreset::ClearDay));

	SetPlayerFrozen(false);
	SetRunState(EPowderRunState::Running);
}

void APowderGameMode::EndRun()
{
	// Save run stats before transitioning to score screen
	if (UPowderGameInstance* GI = Cast<UPowderGameInstance>(GetGameInstance()))
	{
		if (UScoreSubsystem* ScoreSys = GI->GetSubsystem<UScoreSubsystem>())
		{
			GI->OnRunCompleted(ScoreSys->GetCurrentRunStats());
		}
	}

	SetPlayerFrozen(true);
	SetRunState(EPowderRunState::ScoreScreen);
}

void APowderGameMode::OnWipeout()
{
	if (RunState != EPowderRunState::Running)
	{
		return;
	}

	SetRunState(EPowderRunState::WipedOut);
	SetPlayerFrozen(true);

	// Brief delay then respawn at slope start
	GetWorldTimerManager().ClearTimer(WipeoutTimerHandle);
	GetWorldTimerManager().SetTimer(WipeoutTimerHandle, this, &APowderGameMode::RespawnPlayer, 1.5f, false);
}

void APowderGameMode::RespawnPlayer()
{
	// Reset movement and reposition at a nearby course respawn point.
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		if (UPowderMovementComponent* MoveComp = PlayerPawn->FindComponentByClass<UPowderMovementComponent>())
		{
			MoveComp->ResetMovementState();
		}

		if (TerrainManager)
		{
			FVector RespawnPos = TerrainManager->GetRespawnPosition();
			FRotator RespawnRot = TerrainManager->GetRespawnFacingRotation();
			PlayerPawn->SetActorLocationAndRotation(RespawnPos, RespawnRot);
		}
	}

	SetPlayerFrozen(false);
	SetRunState(EPowderRunState::Running);
}

void APowderGameMode::RestartRun()
{
	// Cancel any pending wipeout respawn
	GetWorldTimerManager().ClearTimer(WipeoutTimerHandle);

	// Reset scoring for new run
	if (UScoreSubsystem* ScoreSys = GetGameInstance()->GetSubsystem<UScoreSubsystem>())
	{
		ScoreSys->ResetForNewRun();
	}

	// Reset movement state before repositioning
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		if (UPowderMovementComponent* MoveComp = PlayerPawn->FindComponentByClass<UPowderMovementComponent>())
		{
			MoveComp->ResetMovementState();
		}
	}

	StartRun();
}

void APowderGameMode::OnFinishLineCrossed()
{
	if (RunState == EPowderRunState::Running)
	{
		EndRun();
	}
}

void APowderGameMode::PauseRun()
{
	if (RunState == EPowderRunState::Running)
	{
		SetPlayerFrozen(true);
		SetRunState(EPowderRunState::Paused);
	}
}

void APowderGameMode::ResumeRun()
{
	if (RunState == EPowderRunState::Paused)
	{
		SetPlayerFrozen(false);
		SetRunState(EPowderRunState::Running);
	}
}

void APowderGameMode::QuitToMenu()
{
	// Cancel any pending wipeout respawn
	GetWorldTimerManager().ClearTimer(WipeoutTimerHandle);

	// Reset movement state and reposition at slope start
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		if (UPowderMovementComponent* MoveComp = PlayerPawn->FindComponentByClass<UPowderMovementComponent>())
		{
			MoveComp->ResetMovementState();
		}

		if (TerrainManager)
		{
			FVector StartPos = TerrainManager->GetSlopeStartPosition();
			FRotator StartRot = TerrainManager->GetStartFacingRotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
		}
	}

	SetPlayerFrozen(true);
	SetRunState(EPowderRunState::InMenu);
}

void APowderGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RunState == EPowderRunState::Running)
	{
		UpdateWeatherFromVolumes();
	}
}

void APowderGameMode::UpdateWeatherFromVolumes()
{
	if (!AreRequiredActorsReady())
	{
		return;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	if (!PlayerPawn)
	{
		return;
	}

	FWeatherConfig TargetConfig = UPowderWeatherManager::GetDefaultConfig(EWeatherPreset::ClearDay);
	FWeatherConfig VolumeConfig;
	if (APowderWeatherVolume::GetWeatherAtLocation(GetWorld(), PlayerPawn->GetActorLocation(), VolumeConfig))
	{
		TargetConfig = VolumeConfig;
	}

	ApplyWeatherIfNeeded(TargetConfig);
}

bool APowderGameMode::AreRequiredActorsReady() const
{
	return EnvironmentSetup != nullptr
		&& TerrainManager != nullptr
		&& TerrainManager->IsCourseInitialized();
}

bool APowderGameMode::IsWeatherConfigDifferent(const FWeatherConfig& A, const FWeatherConfig& B) const
{
	constexpr float FloatTolerance = 0.01f;
	constexpr float ColorTolerance = 0.01f;

	if (A.Preset != B.Preset)
	{
		return true;
	}

	auto IsFloatDifferent = [&](float Lhs, float Rhs)
	{
		return !FMath::IsNearlyEqual(Lhs, Rhs, FloatTolerance);
	};

	if (IsFloatDifferent(A.SunIntensity, B.SunIntensity)
		|| IsFloatDifferent(A.SkyLightIntensity, B.SkyLightIntensity)
		|| IsFloatDifferent(A.FogDensity, B.FogDensity)
		|| IsFloatDifferent(A.FogStartDistance, B.FogStartDistance)
		|| IsFloatDifferent(A.FogHeightFalloff, B.FogHeightFalloff)
		|| IsFloatDifferent(A.SnowfallRate, B.SnowfallRate)
		|| IsFloatDifferent(A.WindStrength, B.WindStrength)
		|| IsFloatDifferent(A.WindDirection, B.WindDirection)
		|| IsFloatDifferent(A.TransitionTime, B.TransitionTime))
	{
		return true;
	}

	if (!A.SunColor.Equals(B.SunColor, ColorTolerance)
		|| !A.SkyColor.Equals(B.SkyColor, ColorTolerance)
		|| !A.FogColor.Equals(B.FogColor, ColorTolerance))
	{
		return true;
	}

	return !A.SunRotation.Equals(B.SunRotation, 0.1f);
}

void APowderGameMode::ApplyWeatherIfNeeded(const FWeatherConfig& NewConfig)
{
	if (!EnvironmentSetup)
	{
		return;
	}

	UPowderWeatherManager* WM = EnvironmentSetup->GetWeatherManager();
	if (!WM)
	{
		return;
	}

	if (!bHasLastAppliedWeather)
	{
		WM->SetWeatherImmediate(NewConfig);
		if (USkyLightComponent* SkyLight = EnvironmentSetup->GetSkyLight())
		{
			SkyLight->RecaptureSky();
		}
		LastAppliedWeather = NewConfig;
		bHasLastAppliedWeather = true;
		return;
	}

	if (!IsWeatherConfigDifferent(LastAppliedWeather, NewConfig))
	{
		return;
	}

	WM->SetWeather(NewConfig);
	if (USkyLightComponent* SkyLight = EnvironmentSetup->GetSkyLight())
	{
		SkyLight->RecaptureSky();
	}

	LastAppliedWeather = NewConfig;
	bHasLastAppliedWeather = true;
}

void APowderGameMode::SetRunState(EPowderRunState NewState)
{
	RunState = NewState;
	OnRunStateChanged.Broadcast(NewState);
}

void APowderGameMode::SetPlayerFrozen(bool bFreeze)
{
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		if (UPowderMovementComponent* MoveComp = PlayerPawn->FindComponentByClass<UPowderMovementComponent>())
		{
			MoveComp->SetFrozen(bFreeze);
		}
	}
}
