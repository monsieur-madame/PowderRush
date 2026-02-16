#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraComponent.h"

APowderCharacter::APowderCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule collision
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);
	CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
	SetRootComponent(CapsuleComp);

	// Body mesh (cylinder placeholder)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(CapsuleComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.9f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMeshAsset(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (BodyMeshAsset.Succeeded())
	{
		MeshComp->SetStaticMesh(BodyMeshAsset.Object);
	}

	// Head mesh (sphere placeholder)
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(MeshComp);
	HeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> HeadMeshAsset(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (HeadMeshAsset.Succeeded())
	{
		HeadMesh->SetStaticMesh(HeadMeshAsset.Object);
	}

	// Spring arm for three-quarter diorama camera
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 900.0f;
	SpringArmComp->SetRelativeRotation(FRotator(-45.0f, 30.0f, 0.0f));
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 3.0f;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraRotationLagSpeed = 3.0f;
	SpringArmComp->bDoCollisionTest = false;

	// Camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->FieldOfView = 60.0f;

	// Snow spray particle (Niagara system set in Blueprint)
	SnowSprayComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SnowSpray"));
	SnowSprayComp->SetupAttachment(CapsuleComp);
	SnowSprayComp->SetRelativeLocation(FVector(0.0f, 0.0f, -80.0f));
	SnowSprayComp->SetAutoActivate(false);

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

	UpdateDioramaCamera(DeltaTime);
	UpdateSnowSpray();
}

void APowderCharacter::UpdateDioramaCamera(float DeltaTime)
{
	if (!MovementComp || !SpringArmComp || !CameraComp)
	{
		return;
	}

	float SpeedNorm = MovementComp->GetSpeedNormalized();

	// Arm length: pulls back at speed (900 at rest, 1200 at max speed)
	float TargetArmLength = FMath::Lerp(BaseArmLength, MaxArmLength, SpeedNorm);
	SpringArmComp->TargetArmLength = FMath::FInterpTo(
		SpringArmComp->TargetArmLength, TargetArmLength, DeltaTime, 2.0f);

	// Pitch: lowers at speed to reveal more terrain ahead (-45 at rest, -35 at max)
	float TargetPitch = FMath::Lerp(BasePitch, SpeedPitch, SpeedNorm);

	// Yaw: base offset + tracks carve direction to preview player's path
	float CarveAngle = MovementComp->GetCarveAngle();
	float CarveNorm = CarveAngle / MovementComp->MaxCarveAngle; // -1 to 1
	float TargetYaw = BaseYawOffset + (CarveNorm * CarveYawInfluence);

	FRotator CurrentRot = SpringArmComp->GetRelativeRotation();
	FRotator TargetRot(TargetPitch, TargetYaw, 0.0f);
	SpringArmComp->SetRelativeRotation(
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 3.0f));

	// FOV: subtle widen at speed (60 at rest, 70 at max)
	float TargetFOV = FMath::Lerp(BaseFOV, MaxFOV, SpeedNorm);
	CameraComp->FieldOfView = FMath::FInterpTo(
		CameraComp->FieldOfView, TargetFOV, DeltaTime, 3.0f);
}

void APowderCharacter::UpdateSnowSpray()
{
	if (!MovementComp || !SnowSprayComp)
	{
		return;
	}

	bool bShouldSpray = MovementComp->IsCarving() &&
		MovementComp->GetCurrentSpeed() > MovementComp->MaxSpeed * 0.15f;

	if (bShouldSpray && !SnowSprayComp->IsActive())
	{
		SnowSprayComp->Activate();
	}
	else if (!bShouldSpray && SnowSprayComp->IsActive())
	{
		SnowSprayComp->Deactivate();
	}

	// Orient spray opposite to carve direction
	if (bShouldSpray)
	{
		float CarveAngle = MovementComp->GetCarveAngle();
		float SprayYaw = (CarveAngle > 0.0f) ? -90.0f : 90.0f;
		SnowSprayComp->SetRelativeRotation(FRotator(0.0f, SprayYaw, 0.0f));
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
