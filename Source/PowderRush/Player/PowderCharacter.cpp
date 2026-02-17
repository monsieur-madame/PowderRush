#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Player/PowderTrickComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "Engine/GameInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Effects/PowderSnowSpray.h"
#include "Core/PowderTuningProfile.h"

APowderCharacter::APowderCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule collision
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(CapsuleComp);

	// Skier mesh (pivot is at model center, so Z=0 aligns center-to-center with capsule)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CapsuleComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SkierMeshAsset(
		TEXT("/Game/Skier/Skier.Skier"));
	if (SkierMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(SkierMeshAsset.Object);
	}

	// Spring arm for three-quarter diorama camera
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(CapsuleComp);
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

	// Snow spray (mesh-based particle system)
	SnowSprayComp = CreateDefaultSubobject<UPowderSnowSpray>(TEXT("SnowSpray"));
	SnowSprayComp->SetupAttachment(CapsuleComp);
	SnowSprayComp->SetRelativeLocation(FVector(0.0f, 0.0f, -80.0f));

	// Custom movement
	MovementComp = CreateDefaultSubobject<UPowderMovementComponent>(TEXT("PowderMovement"));
	MovementComp->SetUpdatedComponent(CapsuleComp);

	// Trick system
	TrickComp = CreateDefaultSubobject<UPowderTrickComponent>(TEXT("TrickComponent"));

	// Load default tuning profile
	static ConstructorHelpers::FObjectFinder<UPowderTuningProfile> DefaultProfile(
		TEXT("/Game/TP_DefaultTuningProfile.TP_DefaultTuningProfile"));
	if (DefaultProfile.Succeeded())
	{
		DefaultTuningProfile = DefaultProfile.Object;
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

	// Apply default tuning profile immediately (BlendTime=0 for instant apply on spawn)
	if (DefaultTuningProfile)
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
	float PlayerYaw = GetActorRotation().Yaw;
	float HeadingDelta = FMath::FindDeltaAngleDegrees(DownhillYaw, PlayerYaw);
	float TargetYaw = DownhillYaw + (HeadingDelta * CameraHeadingFollow) + BaseYawOffset;

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
		MovementComp->GetCurrentSpeed() > MovementComp->MaxSpeed * 0.15f;

	if (bShouldSpray)
	{
		float CarveAngle = MovementComp->GetCarveAngle();
		SnowSprayComp->ActivateSpray(CarveAngle);
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

	if (Alpha >= 1.0f)
	{
		bIsBlendingCameraTuning = false;
	}
}
