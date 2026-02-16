#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScoreSubsystem.generated.h"

UENUM(BlueprintType)
enum class EScoreAction : uint8
{
    DeepCarve,
    NearMiss,
    GatePass,
    TrickLanded,
    BoostBurst,
    SpeedBonus,
    AirTimeBonus,
    PowerupCollected
};

USTRUCT(BlueprintType)
struct POWDERRUSH_API FRunStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 TotalScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    float HighestMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 NearMissCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 TricksLanded = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 GatesPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 CoinsCollected = 0;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    float TotalDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "PowderRush|Score")
    int32 BestComboChain = 0;
};

UCLASS()
class POWDERRUSH_API UScoreSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void ResetForNewRun();

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void AddScore(EScoreAction Action, int32 BonusPoints = 0);

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void AddDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void AddCoinCollected();

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void OnWipeout();

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void TickComboTimer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void TickSpeedBonus(float DeltaTime, float SpeedNormalized);

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void AwardAirTimeBonus(float AirTime);

    UFUNCTION(BlueprintCallable, Category = "PowderRush|Score")
    void ActivatePowerupMultiplier(float Multiplier, float Duration);

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    float GetPowerupMultiplier() const { return PowerupMultiplier; }

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    float GetPowerupMultiplierTimeRemaining() const { return PowerupMultiplierTimer; }

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    float GetPowerupMultiplierDuration() const { return PowerupMultiplierDuration; }

    // Queries
    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    int32 GetCurrentScore() const { return CurrentRunStats.TotalScore; }

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    float GetMultiplier() const { return CurrentMultiplier; }

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    int32 GetComboCount() const { return CurrentComboChain; }

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    float GetComboTimerNormalized() const;

    UFUNCTION(BlueprintPure, Category = "PowderRush|Score")
    const FRunStats& GetCurrentRunStats() const { return CurrentRunStats; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreAdded, int32, PointsAdded, int32, TotalScore);
    UPROPERTY(BlueprintAssignable, Category = "PowderRush|Score")
    FOnScoreAdded OnScoreAdded;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplierChanged, float, NewMultiplier);
    UPROPERTY(BlueprintAssignable, Category = "PowderRush|Score")
    FOnMultiplierChanged OnMultiplierChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboDropped);
    UPROPERTY(BlueprintAssignable, Category = "PowderRush|Score")
    FOnComboDropped OnComboDropped;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerupMultiplierChanged, float, NewMultiplier);
    UPROPERTY(BlueprintAssignable, Category = "PowderRush|Score")
    FOnPowerupMultiplierChanged OnPowerupMultiplierChanged;

protected:
    UPROPERTY()
    FRunStats CurrentRunStats;

    float CurrentMultiplier = 1.0f;
    float ComboTimer = 0.0f;
    int32 CurrentComboChain = 0;
    float MaxSpeedTimer = 0.0f;

    float PowerupMultiplier = 1.0f;
    float PowerupMultiplierTimer = 0.0f;
    float PowerupMultiplierDuration = 0.0f;

    static constexpr float ComboTimeout = 2.0f;
    static constexpr float MaxMultiplier = 10.0f;

    int32 GetBasePoints(EScoreAction Action) const;
    float GetMultiplierBonus(EScoreAction Action) const;
    void RefreshComboTimer();
};
