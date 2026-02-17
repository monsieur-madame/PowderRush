// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameMode.h"
#include "Core/PowderEnvironmentSetup.h"
#include "Core/PowderGameInstance.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderPlayerController.h"
#include "Scoring/ScoreSubsystem.h"
#include "UI/PowderHUD.h"
#include "Engine/World.h"
#include "TimerManager.h"

APowderGameMode::APowderGameMode()
{
	DefaultPawnClass = APowderCharacter::StaticClass();
	PlayerControllerClass = APowderPlayerController::StaticClass();
	HUDClass = APowderHUD::StaticClass();
	RunState = EPowderRunState::InMenu;
}

void APowderGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Spawn environment (lighting, sky, fog, trees, rocks, slope material)
	EnvironmentSetup = GetWorld()->SpawnActor<APowderEnvironmentSetup>();

	// Position player at slope start and freeze (InMenu state)
	if (EnvironmentSetup)
	{
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			FVector StartPos = EnvironmentSetup->GetSlopeStartPosition();
			StartPos += EnvironmentSetup->GetSlopeDownhill() * 200.0f;
			StartPos.Z += 100.0f;
			FRotator StartRot = EnvironmentSetup->GetSlopeDownhill().Rotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
		}
	}
	SetPlayerFrozen(true);
}

void APowderGameMode::StartRun()
{
	SetRunState(EPowderRunState::Starting);

	// Move player to the top of the slope
	if (EnvironmentSetup)
	{
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (PlayerPawn)
		{
			FVector StartPos = EnvironmentSetup->GetSlopeStartPosition();
			// Nudge slightly downhill so the player is on the slope, not at the very edge
			StartPos += EnvironmentSetup->GetSlopeDownhill() * 200.0f;
			// Lift above surface so the pawn doesn't clip into the slab
			StartPos.Z += 100.0f;

			FRotator StartRot = EnvironmentSetup->GetSlopeDownhill().Rotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
		}
	}

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
	// Reset movement and reposition at slope start
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (PlayerPawn)
	{
		if (UPowderMovementComponent* MoveComp = PlayerPawn->FindComponentByClass<UPowderMovementComponent>())
		{
			MoveComp->ResetMovementState();
		}

		if (EnvironmentSetup)
		{
			FVector StartPos = EnvironmentSetup->GetSlopeStartPosition();
			StartPos += EnvironmentSetup->GetSlopeDownhill() * 200.0f;
			StartPos.Z += 100.0f;
			FRotator StartRot = EnvironmentSetup->GetSlopeDownhill().Rotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
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

		if (EnvironmentSetup)
		{
			FVector StartPos = EnvironmentSetup->GetSlopeStartPosition();
			StartPos += EnvironmentSetup->GetSlopeDownhill() * 200.0f;
			StartPos.Z += 100.0f;
			FRotator StartRot = EnvironmentSetup->GetSlopeDownhill().Rotation();
			PlayerPawn->SetActorLocationAndRotation(StartPos, StartRot);
		}
	}

	SetPlayerFrozen(true);
	SetRunState(EPowderRunState::InMenu);
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
