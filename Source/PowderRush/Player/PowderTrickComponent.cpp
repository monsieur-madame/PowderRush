#include "Player/PowderTrickComponent.h"
#include "Player/PowderMovementComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

	// Derive TrickPivotHeight from the pelvis bone position in the skeleton
	if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(CachedBodyMesh.Get()))
	{
		// Try the configured bone name, then common alternatives
		static const FName BoneNameCandidates[] = {
			FName(TEXT("pelvis")),
			FName(TEXT("Pelvis")),
			FName(TEXT("Hips")),
			FName(TEXT("hips")),
			FName(TEXT("spine_01")),
		};

		bool bFoundBone = false;

		// Try configured name first
		if (SkelMesh->GetBoneIndex(PivotBoneName) != INDEX_NONE)
		{
			FVector BonePos = SkelMesh->GetBoneLocation(PivotBoneName, EBoneSpaces::ComponentSpace);
			TrickPivotHeight = BonePos.Z;
			bFoundBone = true;
			UE_LOG(LogTemp, Display, TEXT("PowderTrickComponent: Pivot height %.1f from bone '%s'"),
				TrickPivotHeight, *PivotBoneName.ToString());
		}

		// Fallback to common bone name candidates
		if (!bFoundBone)
		{
			for (const FName& Candidate : BoneNameCandidates)
			{
				if (SkelMesh->GetBoneIndex(Candidate) != INDEX_NONE)
				{
					FVector BonePos = SkelMesh->GetBoneLocation(Candidate, EBoneSpaces::ComponentSpace);
					TrickPivotHeight = BonePos.Z;
					PivotBoneName = Candidate;
					bFoundBone = true;
					UE_LOG(LogTemp, Display, TEXT("PowderTrickComponent: Pivot height %.1f from fallback bone '%s'"),
						TrickPivotHeight, *Candidate.ToString());
					break;
				}
			}
		}

		if (!bFoundBone)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("PowderTrickComponent: No pelvis/hips bone found in skeleton. Using default TrickPivotHeight=%.1f"),
				TrickPivotHeight);
		}
	}

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

	// Undo previous frame's pivot offset before movement has a chance to compound it
	UndoPivotOffset();

	TrickTimer += DeltaTime;

	// Interpolate visual rotation on body mesh — composed in heading frame
	// so trick axes are always aligned with movement direction regardless of lean/pitch
	if (CachedBodyMesh && CachedMovement && TrickDuration > 0.0f)
	{
		float Alpha = FMath::Clamp(TrickTimer / TrickDuration, 0.0f, 1.0f);
		// Ease in-out for smoother look
		float EasedAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
		// Component-wise Lerp to preserve full 360° rotations (FRotator::Lerp normalizes to [-180,180])
		FRotator CurrentTrickRot;
		CurrentTrickRot.Pitch = FMath::Lerp(0.0f, TrickTargetRotation.Pitch, EasedAlpha);
		CurrentTrickRot.Yaw = FMath::Lerp(0.0f, TrickTargetRotation.Yaw, EasedAlpha);
		CurrentTrickRot.Roll = FMath::Lerp(0.0f, TrickTargetRotation.Roll, EasedAlpha);

		// Compose: heading → trick → mesh offset
		// Trick rotation happens in the heading-aligned frame, so Pitch = flip, Yaw = spin
		FQuat HeadingQuat = FRotator(0.0f, CachedMovement->GetDesiredYaw(), 0.0f).Quaternion();
		FQuat TrickQuat = CurrentTrickRot.Quaternion();
		FQuat MeshOffsetQuat = FRotator(0.0f, CachedMovement->VisualYawOffset, 0.0f).Quaternion();
		FQuat FullQuat = HeadingQuat * TrickQuat * MeshOffsetQuat;

		// --- Center-of-mass pivot offset ---
		// Rotate around a point TrickPivotHeight above the mesh origin (feet)
		// instead of around the feet themselves.
		// Math: offset = CenterOffset + FullQuat.RotateVector(-CenterOffset)
		// This is zero when no trick rotation is applied (identity trick quat).
		FVector CenterOffset(0.0f, 0.0f, TrickPivotHeight);
		FQuat PivotQuat = HeadingQuat * TrickQuat;  // Pivot uses trick rotation in heading frame (without mesh offset)
		FVector RotatedFoot = PivotQuat.RotateVector(-CenterOffset);
		TrickVisualOffset = CenterOffset + RotatedFoot;

		FVector MeshPos = CachedBodyMesh->GetComponentLocation();
		CachedBodyMesh->SetWorldLocationAndRotation(MeshPos + TrickVisualOffset, FullQuat.Rotator());
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

	// Store spin rotation — applied in heading frame during tick (not relative to current rotation)
	TrickTargetRotation = TrickDef->SpinRotation;

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
	UndoPivotOffset();

	if (CachedBodyMesh && CachedMovement)
	{
		// Compose heading + mesh offset (no tilt — consistent with trick frame)
		FQuat HeadingQuat = FRotator(0.0f, CachedMovement->GetDesiredYaw(), 0.0f).Quaternion();
		FQuat MeshOffsetQuat = FRotator(0.0f, CachedMovement->VisualYawOffset, 0.0f).Quaternion();
		CachedBodyMesh->SetWorldRotation((HeadingQuat * MeshOffsetQuat).Rotator());
	}
}

void UPowderTrickComponent::UndoPivotOffset()
{
	if (CachedBodyMesh && !TrickVisualOffset.IsNearlyZero())
	{
		CachedBodyMesh->SetWorldLocation(CachedBodyMesh->GetComponentLocation() - TrickVisualOffset);
		TrickVisualOffset = FVector::ZeroVector;
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
