// Copyright PowderRush. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/PowderTypes.h"
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
class ATerrainManager;
class UPowderAvalancheComponent;

UCLASS()
class POWDERRUSH_API APowderGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APowderGameMode();

	UFUNCTION(BlueprintPure, Category = "PowderRush|Game")
	ATerrainManager* GetTerrainManager() const { return TerrainManager; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Game")
	APowderEnvironmentSetup* GetEnvironmentSetup() const { return EnvironmentSetup; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Game")
	UPowderAvalancheComponent* GetAvalancheComponent() const { return AvalancheComponent; }

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

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

	void UpdateWeatherFromVolumes();
	bool AreRequiredActorsReady() const;
	bool IsWeatherConfigDifferent(const FWeatherConfig& A, const FWeatherConfig& B) const;
	void ApplyWeatherIfNeeded(const FWeatherConfig& NewConfig);

	UPROPERTY()
	TObjectPtr<APowderEnvironmentSetup> EnvironmentSetup;

	UPROPERTY()
	TObjectPtr<ATerrainManager> TerrainManager;

	UPROPERTY()
	TObjectPtr<UPowderAvalancheComponent> AvalancheComponent;

	bool bRequiredActorsReady = false;
	bool bHasLastAppliedWeather = false;
	FWeatherConfig LastAppliedWeather;
};
