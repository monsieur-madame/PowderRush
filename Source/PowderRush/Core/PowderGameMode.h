// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PowderGameMode.generated.h"

UENUM(BlueprintType)
enum class EPowderRunState : uint8
{
	InMenu,
	Starting,
	Running,
	Paused,
	WipedOut,
	ScoreScreen
};

class APowderEnvironmentSetup;

UCLASS()
class POWDERRUSH_API APowderGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APowderGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void StartRun();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void EndRun();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void OnWipeout();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void RestartRun();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void OnFinishLineCrossed();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void PauseRun();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void ResumeRun();

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Game")
	void QuitToMenu();

	UFUNCTION(BlueprintPure, Category = "PowderRush|Game")
	EPowderRunState GetRunState() const { return RunState; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunStateChanged, EPowderRunState, NewState);

	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Game")
	FOnRunStateChanged OnRunStateChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Game")
	EPowderRunState RunState;

	void SetRunState(EPowderRunState NewState);
	void RespawnPlayer();

	/** Helper to freeze/unfreeze the player's movement component. */
	void SetPlayerFrozen(bool bFreeze);

	FTimerHandle WipeoutTimerHandle;

	UPROPERTY()
	TObjectPtr<APowderEnvironmentSetup> EnvironmentSetup;
};
