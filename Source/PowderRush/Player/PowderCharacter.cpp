#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Effects/PowderSnowSpray.h"
#include "Core/PowderTuningProfile.h"
#include "Core/PowderGameMode.h"
#include "Engine/World.h"

APowderCharacter::APowderCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Skier mesh as root — uses collision set up in the static mesh editor
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetCollisionProfileName(TEXT("Pawn"));
	MeshComp->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	SetRootComponent(MeshComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkierMeshAsset(
		TEXT("/Game/Skier/Skier.Skier"));
	if (SkierMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(SkierMeshAsset.Object);
	}

	// Spring arm for three-quarter diorama camera
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(MeshComp);
	SpringArmComp->TargetArmLength = 900.0f;
	SpringArmComp->SetRelativeRotation(FRotator(-45.0f, 30.0f, 0.0f));
	SpringArmComp->SetUsingAbsoluteRotation(true);
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bEnableCameraLag = false;
	SpringArmComp->bEnableCameraRotationLag = false;
	SpringArmComp->bDoCollisionTest = false;

	// Camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->FieldOfView = 60.0f;

	// Snow spray (Niagara particle system)
	SnowSprayComp = CreateDefaultSubobject<UPowderSnowSpray>(TEXT("SnowSpray"));
	SnowSprayComp->SetupAttachment(MeshComp);
	SnowSprayComp->SetRelativeLocation(FVector(0.0f, 0.0f, -80.0f));

	// Custom movement
	MovementComp = CreateDefaultSubobject<UPowderMovementComponent>(TEXT("PowderMovement"));
	MovementComp->SetUpdatedComponent(MeshComp);

	// Trick system
	TrickComp = CreateDefaultSubobject<UPowderTrickComponent>(TEXT("TrickComponent"));

	// Load default tuning profile
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> DefaultProfile(
		TEXT("/Game/TP_DefaultTuningProfile.TP_DefaultTuningProfile"));
	if (DefaultProfile.Succeeded())
	{
		DefaultTuningProfile = DefaultProfile.Object;
	}

	// C++-only preset ladder wiring (no BP character required).
	FeelPresetLadder.Reset();
	auto AddFeelPreset = [this](const ConstructorHelpers::FObjectFinder<UPowderTuningProfile>& Finder)
	{
		if (Finder.Succeeded() && Finder.Object)
		{
			FeelPresetLadder.Add(Finder.Object);
		}
	};

	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> FeelPreset01(
		TEXT("/Game/TuningProfiles/TP_Feel_Flow_01_Base.TP_Feel_Flow_01_Base"));
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> FeelPreset02(
		TEXT("/Game/TuningProfiles/TP_Feel_Flow_02_QuickerTurnIn.TP_Feel_Flow_02_QuickerTurnIn"));
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> FeelPreset03(
		TEXT("/Game/TuningProfiles/TP_Feel_Flow_03_MoreGrip.TP_Feel_Flow_03_MoreGrip"));
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> FeelPreset04(
		TEXT("/Game/TuningProfiles/TP_Feel_Flow_04_LessSpeedLoss.TP_Feel_Flow_04_LessSpeedLoss"));
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> FeelPreset05(
		TEXT("/Game/TuningProfiles/TP_Feel_Flow_05_CameraLead.TP_Feel_Flow_05_CameraLead"));

	AddFeelPreset(FeelPreset01);
	AddFeelPreset(FeelPreset02);
	AddFeelPreset(FeelPreset03);
	AddFeelPreset(FeelPreset04);
	AddFeelPreset(FeelPreset05);

	if (!DefaultTuningProfile && FeelPresetLadder.Num() > 0)
	{
		DefaultTuningProfile = FeelPresetLadder[0];
	}
}

void APowderCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Wire wipeout to scoring system reset
	if (MovementComp)
	{
		MovementComp->OnWipeout.AddDynamic(this, &APowderCharacter::HandleWipeout);
	}

	// Prefer feel preset ladder when configured; otherwise fall back to default profile.
	if (FeelPresetLadder.Num() > 0)
	{
		ApplyFeelPresetByIndex(0, 0.0f);
	}
	else if (DefaultTuningProfile)
	{
		// Apply movement values directly (no blend on startup)
		if (MovementComp)
		{
			MovementComp->ApplyTuningProfile(DefaultTuningProfile->Movement, 0.0f);
		}
		ApplyCameraTuning(DefaultTuningProfile->Camera, 0.0f);
	}
}

void APowderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickCameraTuningBlend(DeltaTime);
	UpdateDioramaCamera(DeltaTime);
	UpdateSnowSpray();

	// Drive scoring ticks
	if (UScoreSubsystem* ScoreSys = GetGameInstance()->GetSubsystem<UScoreSubsystem>())
	{
		ScoreSys->TickComboTimer(DeltaTime);
		ScoreSys->TickSpeedBonus(DeltaTime, MovementComp->GetSpeedNormalized());
	}
}

void APowderCharacter::HandleWipeout()
{
	if (UScoreSubsystem* ScoreSys = GetGameInstance()->GetSubsystem<UScoreSubsystem>())
	{
		ScoreSys->OnWipeout();
	}

	// Notify GameMode for crash/respawn flow
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnWipeout();
	}
}

void APowderCharacter::UpdateDioramaCamera(float DeltaTime)
{
	if (!MovementComp || !SpringArmComp || !CameraComp)
	{
		return;
	}

	float SpeedNorm = MovementComp->GetSpeedNormalized();

	// Arm length: pulls back at speed
	float TargetArmLength = FMath::Lerp(BaseArmLength, MaxArmLength, SpeedNorm);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength, TargetArmLength, DeltaTime, ArmLengthInterpSpeed);

	// Camera yaw: blend between downhill and player heading, with interp lag
	float DownhillYaw = MovementComp->GetSlopeForwardYaw();
	float DesiredHeadingYaw = MovementComp->GetDesiredYaw();
	float HeadingDelta = FMath::FindDeltaAngleDegrees(DownhillYaw, DesiredHeadingYaw);
	float LeadYaw = DownhillYaw + (HeadingDelta * CameraTurnLeadWeight);
	float LookAheadYaw = FMath::Clamp(
		HeadingDelta * SpeedNorm * CameraLookAheadWeight,
		-CameraLookAheadMaxYaw,
		CameraLookAheadMaxYaw);
	float FollowYaw = DownhillYaw + (HeadingDelta * CameraHeadingFollow);
	float TargetYaw = FMath::Lerp(FollowYaw, LeadYaw + LookAheadYaw, FMath::Clamp(CameraTurnLeadWeight, 0.0f, 1.0f)) + BaseYawOffset;

	float TargetPitch = FMath::Lerp(BasePitch, SpeedPitch, SpeedNorm);

	// Use world rotation so the spring arm isn't compounded with the capsule's yaw
	FRotator CurrentRot = SpringArmComp->GetComponentRotation();
	FRotator TargetRot(TargetPitch, TargetYaw, 0.0f);
	SpringArmComp->SetWorldRotation(
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CameraYawInterpSpeed));

	// FOV: subtle widen at speed
	float TargetFOV = FMath::Lerp(BaseFOV, MaxFOV, SpeedNorm);
	CameraComp->FieldOfView = FMath::FInterpTo(
		CameraComp->FieldOfView, TargetFOV, DeltaTime, FOVInterpSpeed);
}

void APowderCharacter::UpdateSnowSpray()
{
	if (!MovementComp || !SnowSprayComp)
	{
		return;
	}

	bool bShouldSpray = MovementComp->IsCarving() &&
		MovementComp->GetCurrentSpeed() > MovementComp->MaxSpeed * 0.05f;

	if (bShouldSpray)
	{
		float CarveAngle = MovementComp->GetCarveAngle();
		const FSurfaceProperties& Surface = MovementComp->CurrentSurface;
		SnowSprayComp->ActivateSpray(CarveAngle, Surface.SnowSprayAmount, Surface.SprayColor);
	}
	else if (!bShouldSpray && SnowSprayComp->IsSprayActive())
	{
		SnowSprayComp->DeactivateSpray();
	}
}

void APowderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Input is handled by PowderPlayerController via Enhanced Input
}

UPawnMovementComponent* APowderCharacter::GetMovementComponent() const
{
	return MovementComp.Get();
}

void APowderCharacter::ApplyTuningProfile(const UPowderTuningProfile* Profile)
{
	if (!Profile)
	{
		return;
	}

	if (MovementComp)
	{
		MovementComp->ApplyTuningProfile(Profile->Movement, Profile->BlendTime);
	}

	ApplyCameraTuning(Profile->Camera, Profile->BlendTime);
}

bool APowderCharacter::ApplyFeelPresetByIndex(int32 NewIndex, float OverrideBlendTime)
{
	if (!FeelPresetLadder.IsValidIndex(NewIndex))
	{
		return false;
	}

	UPowderTuningProfile* Profile = FeelPresetLadder[NewIndex];
	if (!Profile)
	{
		return false;
	}

	const int32 PriorActive = ActiveFeelPresetIndex;
	if (PriorActive != NewIndex)
	{
		PreviousFeelPresetIndex = PriorActive;
		ActiveFeelPresetIndex = NewIndex;
	}

	const float BlendTime = (OverrideBlendTime >= 0.0f) ? OverrideBlendTime : Profile->BlendTime;
	if (MovementComp)
	{
		MovementComp->ApplyTuningProfile(Profile->Movement, BlendTime);
	}
	ApplyCameraTuning(Profile->Camera, BlendTime);
	return true;
}

bool APowderCharacter::StepFeelPreset(int32 Direction)
{
	if (FeelPresetLadder.Num() == 0)
	{
		return false;
	}

	const int32 Step = (Direction >= 0) ? 1 : -1;
	int32 NextIndex = ActiveFeelPresetIndex;
	if (!FeelPresetLadder.IsValidIndex(NextIndex))
	{
		NextIndex = 0;
	}
	else
	{
		NextIndex = (NextIndex + Step + FeelPresetLadder.Num()) % FeelPresetLadder.Num();
	}

	return ApplyFeelPresetByIndex(NextIndex, 0.25f);
}

bool APowderCharacter::ToggleLastFeelPreset()
{
	if (!FeelPresetLadder.IsValidIndex(ActiveFeelPresetIndex) || !FeelPresetLadder.IsValidIndex(PreviousFeelPresetIndex))
	{
		return false;
	}

	const int32 SwapIndex = PreviousFeelPresetIndex;
	return ApplyFeelPresetByIndex(SwapIndex, 0.2f);
}

FString APowderCharacter::GetActiveFeelPresetName() const
{
	if (!FeelPresetLadder.IsValidIndex(ActiveFeelPresetIndex) || !FeelPresetLadder[ActiveFeelPresetIndex])
	{
		return TEXT("None");
	}

	return FeelPresetLadder[ActiveFeelPresetIndex]->GetName();
}

void APowderCharacter::ApplyCameraTuning(const FCameraTuning& Tuning, float BlendTime)
{
	// Snapshot current values
	CameraTuningBlendStart.BaseArmLength = BaseArmLength;
	CameraTuningBlendStart.MaxArmLength = MaxArmLength;
	CameraTuningBlendStart.ArmLengthInterpSpeed = ArmLengthInterpSpeed;
	CameraTuningBlendStart.BasePitch = BasePitch;
	CameraTuningBlendStart.SpeedPitch = SpeedPitch;
	CameraTuningBlendStart.BaseYawOffset = BaseYawOffset;
	CameraTuningBlendStart.CameraHeadingFollow = CameraHeadingFollow;
	CameraTuningBlendStart.CameraYawInterpSpeed = CameraYawInterpSpeed;
	CameraTuningBlendStart.BaseFOV = BaseFOV;
	CameraTuningBlendStart.MaxFOV = MaxFOV;
	CameraTuningBlendStart.FOVInterpSpeed = FOVInterpSpeed;
	CameraTuningBlendStart.CameraTurnLeadWeight = CameraTurnLeadWeight;
	CameraTuningBlendStart.CameraLookAheadWeight = CameraLookAheadWeight;
	CameraTuningBlendStart.CameraLookAheadMaxYaw = CameraLookAheadMaxYaw;

	CameraTuningBlendTarget = Tuning;
	CameraTuningBlendDuration = FMath::Max(BlendTime, KINDA_SMALL_NUMBER);
	CameraTuningBlendAlpha = 0.0f;
	bIsBlendingCameraTuning = true;
}

void APowderCharacter::TickCameraTuningBlend(float DeltaTime)
{
	if (!bIsBlendingCameraTuning)
	{
		return;
	}

	CameraTuningBlendAlpha = FMath::Clamp(CameraTuningBlendAlpha + DeltaTime / CameraTuningBlendDuration, 0.0f, 1.0f);
	float Alpha = CameraTuningBlendAlpha;

	BaseArmLength = FMath::Lerp(CameraTuningBlendStart.BaseArmLength, CameraTuningBlendTarget.BaseArmLength, Alpha);
	MaxArmLength = FMath::Lerp(CameraTuningBlendStart.MaxArmLength, CameraTuningBlendTarget.MaxArmLength, Alpha);
	ArmLengthInterpSpeed = FMath::Lerp(CameraTuningBlendStart.ArmLengthInterpSpeed, CameraTuningBlendTarget.ArmLengthInterpSpeed, Alpha);
	BasePitch = FMath::Lerp(CameraTuningBlendStart.BasePitch, CameraTuningBlendTarget.BasePitch, Alpha);
	SpeedPitch = FMath::Lerp(CameraTuningBlendStart.SpeedPitch, CameraTuningBlendTarget.SpeedPitch, Alpha);
	BaseYawOffset = FMath::Lerp(CameraTuningBlendStart.BaseYawOffset, CameraTuningBlendTarget.BaseYawOffset, Alpha);
	CameraHeadingFollow = FMath::Lerp(CameraTuningBlendStart.CameraHeadingFollow, CameraTuningBlendTarget.CameraHeadingFollow, Alpha);
	CameraYawInterpSpeed = FMath::Lerp(CameraTuningBlendStart.CameraYawInterpSpeed, CameraTuningBlendTarget.CameraYawInterpSpeed, Alpha);
	BaseFOV = FMath::Lerp(CameraTuningBlendStart.BaseFOV, CameraTuningBlendTarget.BaseFOV, Alpha);
	MaxFOV = FMath::Lerp(CameraTuningBlendStart.MaxFOV, CameraTuningBlendTarget.MaxFOV, Alpha);
	FOVInterpSpeed = FMath::Lerp(CameraTuningBlendStart.FOVInterpSpeed, CameraTuningBlendTarget.FOVInterpSpeed, Alpha);
	CameraTurnLeadWeight = FMath::Lerp(CameraTuningBlendStart.CameraTurnLeadWeight, CameraTuningBlendTarget.CameraTurnLeadWeight, Alpha);
	CameraLookAheadWeight = FMath::Lerp(CameraTuningBlendStart.CameraLookAheadWeight, CameraTuningBlendTarget.CameraLookAheadWeight, Alpha);
	CameraLookAheadMaxYaw = FMath::Lerp(CameraTuningBlendStart.CameraLookAheadMaxYaw, CameraTuningBlendTarget.CameraLookAheadMaxYaw, Alpha);

	if (Alpha >= 1.0f)
	{
		bIsBlendingCameraTuning = false;
	}
}
