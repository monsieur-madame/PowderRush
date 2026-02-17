#include "Pickup/PowderPowerup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Scoring/ScoreSubsystem.h"
#include "UI/PowderHUD.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

APowderPowerup::APowderPowerup()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collection sphere
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetSphereRadius(100.0f);
	CollectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetGenerateOverlapEvents(true);
	SetRootComponent(CollectionSphere);

	// Visual mesh (sphere placeholder, Blueprint-overridable)
	PowerupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PowerupMesh"));
	PowerupMesh->SetupAttachment(CollectionSphere);
	PowerupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PowerupMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (MeshAsset.Succeeded())
	{
		PowerupMesh->SetStaticMesh(MeshAsset.Object);
	}

	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &APowderPowerup::OnOverlapBegin);
}

void APowderPowerup::BeginPlay()
{
	Super::BeginPlay();
	InitialZ = GetActorLocation().Z;
	TimeAccumulator = FMath::FRand() * UE_TWO_PI;
}

void APowderPowerup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected)
	{
		return;
	}

	// Rotate
	FRotator Rotation = PowerupMesh->GetRelativeRotation();
	Rotation.Yaw += RotationSpeed * DeltaTime;
	PowerupMesh->SetRelativeRotation(Rotation);

	// Bob
	TimeAccumulator += DeltaTime * BobSpeed;
	FVector Location = GetActorLocation();
	Location.Z = InitialZ + FMath::Sin(TimeAccumulator) * BobAmplitude;
	SetActorLocation(Location);
}

void APowderPowerup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected)
	{
		return;
	}

	if (Cast<APowderCharacter>(OtherActor))
	{
		Collect(OtherActor);
	}
}

void APowderPowerup::Collect(AActor* Collector)
{
	bCollected = true;

	APowderCharacter* Character = Cast<APowderCharacter>(Collector);
	if (!Character)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UScoreSubsystem* ScoreSys = GI ? GI->GetSubsystem<UScoreSubsystem>() : nullptr;

	// Award collection points and track powerup stat
	if (ScoreSys)
	{
		ScoreSys->AddScore(EScoreAction::PowerupCollected, CollectPoints);
		ScoreSys->AddPowerupCollected();
	}

	// Apply effect based on type
	switch (PowerupType)
	{
	case EPowerupType::SpeedBoost:
		if (UPowderMovementComponent* MoveComp = Character->GetPowderMovement())
		{
			MoveComp->TriggerSpeedBoost(BoostSpeed, BoostDuration);
		}
		// Notify HUD
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (APowderHUD* HUD = Cast<APowderHUD>(PC->GetHUD()))
			{
				HUD->ShowPowerupIndicator(PowerupType, BoostDuration);
			}
		}
		break;

	case EPowerupType::ScoreMultiplier:
		if (ScoreSys)
		{
			ScoreSys->ActivatePowerupMultiplier(ScoreMultiplier, MultiplierDuration);
		}
		// Notify HUD
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (APowderHUD* HUD = Cast<APowderHUD>(PC->GetHUD()))
			{
				HUD->ShowPowerupIndicator(PowerupType, MultiplierDuration);
			}
		}
		break;
	}

	// Hide and cleanup
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetLifeSpan(1.0f);
}
