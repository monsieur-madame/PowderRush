#include "Scoring/ScoreSubsystem.h"

void UScoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ResetForNewRun();
}

void UScoreSubsystem::ResetForNewRun()
{
    CurrentRunStats = FRunStats();
    CurrentMultiplier = 1.0f;
    ComboTimer = 0.0f;
    CurrentComboChain = 0;
    MaxSpeedTimer = 0.0f;
}

void UScoreSubsystem::AddScore(EScoreAction Action, int32 BonusPoints)
{
    int32 BasePoints = GetBasePoints(Action) + BonusPoints;
    int32 FinalPoints = FMath::RoundToInt32(BasePoints * CurrentMultiplier);
    CurrentRunStats.TotalScore += FinalPoints;

    // Update multiplier
    float MultiplierBonus = GetMultiplierBonus(Action);
    if (MultiplierBonus > 0.0f)
    {
        float OldMultiplier = CurrentMultiplier;
        CurrentMultiplier = FMath::Min(CurrentMultiplier + MultiplierBonus, MaxMultiplier);
        if (CurrentMultiplier != OldMultiplier)
        {
            OnMultiplierChanged.Broadcast(CurrentMultiplier);
        }
    }

    // Update per-action stats
    switch (Action)
    {
    case EScoreAction::NearMiss:
        CurrentRunStats.NearMissCount++;
        break;
    case EScoreAction::TrickLanded:
        CurrentRunStats.TricksLanded++;
        break;
    case EScoreAction::GatePass:
        CurrentRunStats.GatesPassed++;
        break;
    default:
        break;
    }

    // Refresh combo
    CurrentComboChain++;
    CurrentRunStats.BestComboChain = FMath::Max(CurrentRunStats.BestComboChain, CurrentComboChain);
    RefreshComboTimer();

    // Track highest multiplier
    CurrentRunStats.HighestMultiplier = FMath::Max(CurrentRunStats.HighestMultiplier, CurrentMultiplier);

    OnScoreAdded.Broadcast(FinalPoints, CurrentRunStats.TotalScore);
}

void UScoreSubsystem::AddDistance(float Distance)
{
    CurrentRunStats.TotalDistance += Distance;
}

void UScoreSubsystem::AddCoinCollected()
{
    CurrentRunStats.CoinsCollected++;
}

void UScoreSubsystem::OnWipeout()
{
    CurrentMultiplier = 1.0f;
    CurrentComboChain = 0;
    ComboTimer = 0.0f;
    OnMultiplierChanged.Broadcast(CurrentMultiplier);
    OnComboDropped.Broadcast();
}

void UScoreSubsystem::TickComboTimer(float DeltaTime)
{
    if (ComboTimer > 0.0f)
    {
        ComboTimer -= DeltaTime;
        if (ComboTimer <= 0.0f)
        {
            // Combo expired
            ComboTimer = 0.0f;
            CurrentComboChain = 0;
            CurrentMultiplier = 1.0f;
            OnMultiplierChanged.Broadcast(CurrentMultiplier);
            OnComboDropped.Broadcast();
        }
    }
}

void UScoreSubsystem::TickSpeedBonus(float DeltaTime, float SpeedNormalized)
{
    if (SpeedNormalized >= 0.9f)
    {
        MaxSpeedTimer += DeltaTime;
        if (MaxSpeedTimer >= 3.0f)
        {
            AddScore(EScoreAction::SpeedBonus);
            MaxSpeedTimer = 0.0f;
        }
    }
    else
    {
        MaxSpeedTimer = 0.0f;
    }
}

void UScoreSubsystem::AwardAirTimeBonus(float AirTime)
{
    if (AirTime >= 1.0f)
    {
        int32 Bonus = FMath::FloorToInt32(AirTime) * 25;
        AddScore(EScoreAction::AirTimeBonus, Bonus);
    }
}

float UScoreSubsystem::GetComboTimerNormalized() const
{
    return ComboTimeout > 0.0f ? FMath::Clamp(ComboTimer / ComboTimeout, 0.0f, 1.0f) : 0.0f;
}

int32 UScoreSubsystem::GetBasePoints(EScoreAction Action) const
{
    switch (Action)
    {
    case EScoreAction::DeepCarve:   return 10;
    case EScoreAction::NearMiss:    return 50;
    case EScoreAction::GatePass:    return 25;
    case EScoreAction::TrickLanded: return 100; // BonusPoints adds up to 400 more
    case EScoreAction::BoostBurst:  return 30;
    case EScoreAction::SpeedBonus:  return 50;
    case EScoreAction::AirTimeBonus: return 0;  // All via BonusPoints
    default:                        return 0;
    }
}

float UScoreSubsystem::GetMultiplierBonus(EScoreAction Action) const
{
    switch (Action)
    {
    case EScoreAction::NearMiss:     return 0.5f;
    case EScoreAction::GatePass:     return 0.2f;
    case EScoreAction::TrickLanded:  return 1.0f;
    case EScoreAction::AirTimeBonus: return 0.2f;
    default:                         return 0.0f;
    }
}

void UScoreSubsystem::RefreshComboTimer()
{
    ComboTimer = ComboTimeout;
}
