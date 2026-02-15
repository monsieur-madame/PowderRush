#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

APowderCharacter::APowderCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule collision
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(CapsuleComp);

	// Skeletal mesh for character model
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CapsuleComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));

	// Spring arm for camera
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 400.0f;
	SpringArmComp->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 5.0f;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraRotationLagSpeed = 5.0f;

	// Camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// Custom movement
	MovementComp = CreateDefaultSubobject<UPowderMovementComponent>(TEXT("PowderMovement"));
	MovementComp->SetUpdatedComponent(CapsuleComp);
}

void APowderCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APowderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Dynamic camera: pull back at speed, push in during carves
	if (MovementComp && SpringArmComp)
	{
		float SpeedNorm = MovementComp->GetSpeedNormalized();

		// Arm length: 300 at rest, 600 at max speed
		float TargetArmLength = FMath::Lerp(300.0f, 600.0f, SpeedNorm);
		SpringArmComp->TargetArmLength = FMath::FInterpTo(
			SpringArmComp->TargetArmLength, TargetArmLength, DeltaTime, 3.0f);

		// Camera pitch: -15 at rest, -25 at max speed
		float TargetPitch = FMath::Lerp(-15.0f, -25.0f, SpeedNorm);

		// Camera roll tilt into carves
		float CarveAngle = MovementComp->GetCarveAngle();
		float TargetRoll = CarveAngle * 0.1f;

		FRotator CurrentRot = SpringArmComp->GetRelativeRotation();
		FRotator TargetRot(TargetPitch, CurrentRot.Yaw, TargetRoll);
		SpringArmComp->SetRelativeRotation(
			FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 4.0f));
	}
}

void APowderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Input is handled by PowderPlayerController via Enhanced Input
}

UPawnMovementComponent* APowderCharacter::GetMovementComponent() const
{
	return MovementComp;
}
