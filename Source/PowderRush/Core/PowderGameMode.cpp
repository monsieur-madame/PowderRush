// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameMode.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderPlayerController.h"

APowderGameMode::APowderGameMode()
{
	DefaultPawnClass = APowderCharacter::StaticClass();
	PlayerControllerClass = APowderPlayerController::StaticClass();
	RunState = EPowderRunState::InMenu;
}

void APowderGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Auto-start run for prototyping — skip menu flow
	StartRun();
}

void APowderGameMode::StartRun()
{
	SetRunState(EPowderRunState::Starting);

	// TODO: Reset terrain, score, position

	SetRunState(EPowderRunState::Running);
}

void APowderGameMode::EndRun()
{
	SetRunState(EPowderRunState::ScoreScreen);
}

void APowderGameMode::OnWipeout()
{
	SetRunState(EPowderRunState::WipedOut);

	// Brief delay then show score
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &APowderGameMode::EndRun, 2.0f, false);
}

void APowderGameMode::RestartRun()
{
	StartRun();
}

void APowderGameMode::SetRunState(EPowderRunState NewState)
{
	RunState = NewState;
	OnRunStateChanged.Broadcast(NewState);
}
