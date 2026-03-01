#include "Player/PowderTrickComponent.h"
#include "Player/PowderMovementComponent.h"
#include "Components/MeshComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "Engine/GameInstance.h"

UPowderTrickComponent::UPowderTrickComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default trick registry
	{
		FPowderTrickDefinition Trick;
		Trick.TrickType = EPowderTrickType::Backflip;
		Trick.RequiredGesture = EPowderGestureDirection::Up;
		Trick.BasePoints = 100;
		Trick.Duration = 0.5f;
		Trick.SpinRotation = FRotator(-360.0f, 0.0f, 0.0f);
		Trick.DisplayName = FName(TEXT("BACKFLIP"));
		TrickDefinitions.Add(Trick);
	}
	{
		FPowderTrickDefinition Trick;
		Trick.TrickType = EPowderTrickType::Frontflip;
		Trick.RequiredGesture = EPowderGestureDirection::Down;
		Trick.BasePoints = 100;
		Trick.Duration = 0.5f;
		Trick.SpinRotation = FRotator(360.0f, 0.0f, 0.0f);
		Trick.DisplayName = FName(TEXT("FRONTFLIP"));
		TrickDefinitions.Add(Trick);
	}
	{
		FPowderTrickDefinition Trick;
		Trick.TrickType = EPowderTrickType::HeliSpinLeft;
		Trick.RequiredGesture = EPowderGestureDirection::Left;
		Trick.BasePoints = 150;
		Trick.Duration = 0.6f;
		Trick.SpinRotation = FRotator(0.0f, -360.0f, 0.0f);
		Trick.DisplayName = FName(TEXT("HELI SPIN"));
		TrickDefinitions.Add(Trick);
	}
	{
		FPowderTrickDefinition Trick;
		Trick.TrickType = EPowderTrickType::HeliSpinRight;
		Trick.RequiredGesture = EPowderGestureDirection::Right;
		Trick.BasePoints = 150;
		Trick.Duration = 0.6f;
		Trick.SpinRotation = FRotator(0.0f, 360.0f, 0.0f);
		Trick.DisplayName = FName(TEXT("HELI SPIN"));
		TrickDefinitions.Add(Trick);
	}
	{
		FPowderTrickDefinition Trick;
		Trick.TrickType = EPowderTrickType::SpreadEagle;
		Trick.RequiredGesture = EPowderGestureDirection::HoldBoth;
		Trick.BasePoints = 200;
		Trick.Duration = 0.8f;
		Trick.SpinRotation = FRotator(0.0f, 0.0f, 30.0f);
		Trick.DisplayName = FName(TEXT("SPREAD EAGLE"));
		TrickDefinitions.Add(Trick);
	}
}

void UPowderTrickComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache references
	CachedMovement = GetOwner()->FindComponentByClass<UPowderMovementComponent>();
	CachedBodyMesh = GetOwner()->FindComponentByClass<UMeshComponent>();

	// Bind to movement delegates
	if (CachedMovement)
	{
		CachedMovement->OnLaunched.AddDynamic(this, &UPowderTrickComponent::OnBecameAirborne);
		CachedMovement->OnLanded.AddDynamic(this, &UPowderTrickComponent::OnLanded);
	}
}

void UPowderTrickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState != EPowderTrickState::Executing)
	{
		return;
	}

	TrickTimer += DeltaTime;

	// Interpolate visual rotation on body mesh
	if (CachedBodyMesh && TrickDuration > 0.0f)
	{
		float Alpha = FMath::Clamp(TrickTimer / TrickDuration, 0.0f, 1.0f);
		// Ease in-out for smoother look
		float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
		// Component-wise Lerp to preserve full 360° rotations (FRotator::Lerp normalizes to [-180,180])
		FRotator CurrentTrickRot;
		CurrentTrickRot.Pitch = FMath::Lerp(TrickStartRotation.Pitch, TrickTargetRotation.Pitch, EasedAlpha);
		CurrentTrickRot.Yaw = FMath::Lerp(TrickStartRotation.Yaw, TrickTargetRotation.Yaw, EasedAlpha);
		CurrentTrickRot.Roll = FMath::Lerp(TrickStartRotation.Roll, TrickTargetRotation.Roll, EasedAlpha);
		CachedBodyMesh->SetRelativeRotation(CurrentTrickRot);
	}

	// Check completion
	if (TrickTimer >= TrickDuration)
	{
		CompleteTrick();
	}
}

bool UPowderTrickComponent::RequestTrick(EPowderGestureDirection Gesture)
{
	// Can only trick while airborne
	if (!CachedMovement || !CachedMovement->IsAirborne())
	{
		return false;
	}

	// Can't start a new trick while one is executing
	if (CurrentState == EPowderTrickState::Executing)
	{
		return false;
	}

	const FPowderTrickDefinition* TrickDef = FindTrickForGesture(Gesture);
	if (!TrickDef)
	{
		return false;
	}

	// Start the trick
	CurrentState = EPowderTrickState::Executing;
	ActiveTrickType = TrickDef->TrickType;
	TrickTimer = 0.0f;
	TrickDuration = TrickDef->Duration;

	// Set up visual rotation
	TrickStartRotation = CachedBodyMesh ? CachedBodyMesh->GetRelativeRotation() : FRotator::ZeroRotator;
	TrickTargetRotation = TrickStartRotation + TrickDef->SpinRotation;

	return true;
}

void UPowderTrickComponent::CompleteTrick()
{
	// Find the definition for scoring
	const FPowderTrickDefinition* TrickDef = nullptr;
	for (const FPowderTrickDefinition& Def : TrickDefinitions)
	{
		if (Def.TrickType == ActiveTrickType)
		{
			TrickDef = &Def;
			break;
		}
	}

	int32 Points = TrickDef ? TrickDef->BasePoints : 100;
	int32 ChainedPoints = FMath::RoundToInt32(Points * ChainMultiplier);

	CurrentJumpTricks.Add(ActiveTrickType);
	TotalJumpPoints += ChainedPoints;
	TrickChainCount++;
	ChainMultiplier *= 1.5f;

	CurrentState = EPowderTrickState::Completed;
	OnTrickCompleted.Broadcast(ActiveTrickType, ChainedPoints);

	// Reset body mesh rotation to movement-driven heading
	RestoreBaseRotation();

	// Ready for next trick in chain (stays airborne)
	ActiveTrickType = EPowderTrickType::None;
}

void UPowderTrickComponent::FailTrick()
{
	CurrentState = EPowderTrickState::Failed;
	OnTrickFailed.Broadcast();

	// Reset body mesh rotation to movement-driven heading
	RestoreBaseRotation();

	// Trigger wipeout on movement component
	if (CachedMovement)
	{
		CachedMovement->TriggerWipeout();
	}
}

void UPowderTrickComponent::OnBecameAirborne()
{
	ResetJumpState();
}

void UPowderTrickComponent::OnLanded(float AirTime, float LandingQuality)
{
	// If mid-trick on landing, that's a fail
	if (CurrentState == EPowderTrickState::Executing)
	{
		FailTrick();
		// Build result with failed flag
		LastJumpResult.TricksPerformed = CurrentJumpTricks;
		LastJumpResult.TotalPoints = TotalJumpPoints;
		LastJumpResult.bAllCompleted = false;
		ResetJumpState();
		return;
	}

	// Build successful result
	LastJumpResult.TricksPerformed = CurrentJumpTricks;
	LastJumpResult.TotalPoints = TotalJumpPoints;
	LastJumpResult.bAllCompleted = true;

	// Award trick points to scoring system
	if (TotalJumpPoints > 0)
	{
		if (UGameInstance* GI = GetOwner()->GetGameInstance())
		{
			if (UScoreSubsystem* ScoreSys = GI->GetSubsystem<UScoreSubsystem>())
			{
				ScoreSys->AddScore(EScoreAction::TrickLanded, TotalJumpPoints);
			}
		}
	}

	// Award airtime bonus
	if (AirTime >= 1.0f)
	{
		if (UGameInstance* GI = GetOwner()->GetGameInstance())
		{
			if (UScoreSubsystem* ScoreSys = GI->GetSubsystem<UScoreSubsystem>())
			{
				ScoreSys->AwardAirTimeBonus(AirTime);
			}
		}
	}

	ResetJumpState();
}

void UPowderTrickComponent::ResetJumpState()
{
	CurrentState = EPowderTrickState::Idle;
	ActiveTrickType = EPowderTrickType::None;
	TrickTimer = 0.0f;
	TrickDuration = 0.0f;
	TrickChainCount = 0;
	ChainMultiplier = 1.0f;
	CurrentJumpTricks.Empty();
	TotalJumpPoints = 0;

	// Reset body mesh rotation to movement-driven heading
	RestoreBaseRotation();
}

void UPowderTrickComponent::RestoreBaseRotation()
{
	if (CachedBodyMesh && CachedMovement)
	{
		CachedBodyMesh->SetWorldRotation(FRotator(0.0f, CachedMovement->GetDesiredYaw() + CachedMovement->VisualYawOffset, 0.0f));
	}
}

const FPowderTrickDefinition* UPowderTrickComponent::FindTrickForGesture(EPowderGestureDirection Gesture) const
{
	for (const FPowderTrickDefinition& Def : TrickDefinitions)
	{
		if (Def.RequiredGesture == Gesture)
		{
			return &Def;
		}
	}
	return nullptr;
}
