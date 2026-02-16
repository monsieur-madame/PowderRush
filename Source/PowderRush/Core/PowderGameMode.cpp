// Copyright PowderRush. All Rights Reserved.

#include "Core/PowderGameMode.h"
#include "Core/PowderEnvironmentSetup.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderPlayerController.h"
#include "UI/PowderHUD.h"

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

	// Auto-start run for prototyping — skip menu flow
	StartRun();
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
