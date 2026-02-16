#include "Pickup/CoinPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "NiagaraComponent.h"
#include "Player/PowderCharacter.h"
#include "Scoring/ScoreSubsystem.h"
#include "Core/PowderGameInstance.h"
#include "Engine/GameInstance.h"

ACoinPickup::ACoinPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collection sphere
	CollectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphere"));
	CollectionSphere->SetSphereRadius(80.0f);
	CollectionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollectionSphere->SetGenerateOverlapEvents(true);
	SetRootComponent(CollectionSphere);

	// Coin mesh (flattened sphere = disc shape)
	CoinMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoinMesh"));
	CoinMesh->SetupAttachment(CollectionSphere);
	CoinMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoinMesh->SetRelativeScale3D(FVector(0.5f, 0.15f, 0.5f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CoinMeshAsset(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (CoinMeshAsset.Succeeded())
	{
		CoinMesh->SetStaticMesh(CoinMeshAsset.Object);
	}

	// Sparkle effect attachment point
	SparkleEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SparkleEffect"));
	SparkleEffect->SetupAttachment(CoinMesh);
	SparkleEffect->SetAutoActivate(true);

	CollectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACoinPickup::OnOverlapBegin);
}

void ACoinPickup::BeginPlay()
{
	Super::BeginPlay();
	InitialZ = GetActorLocation().Z;
	TimeAccumulator = FMath::FRand() * UE_TWO_PI; // Random phase offset so coins don't bob in sync
}

void ACoinPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCollected)
	{
		return;
	}

	// Rotate coin
	FRotator Rotation = CoinMesh->GetRelativeRotation();
	Rotation.Yaw += RotationSpeed * DeltaTime;
	CoinMesh->SetRelativeRotation(Rotation);

	// Bob up and down
	TimeAccumulator += DeltaTime * BobSpeed;
	FVector Location = GetActorLocation();
	Location.Z = InitialZ + FMath::Sin(TimeAccumulator) * BobAmplitude;
	SetActorLocation(Location);
}

void ACoinPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected)
	{
		return;
	}

	APowderCharacter* Player = Cast<APowderCharacter>(OtherActor);
	if (Player)
	{
		Collect(OtherActor);
	}
}

void ACoinPickup::Collect(AActor* Collector)
{
	bCollected = true;

	// Add coin to game instance
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPowderGameInstance* PowderGI = Cast<UPowderGameInstance>(GI))
		{
			PowderGI->AddCoins(CoinValue);
		}

		// Track in scoring
		if (UScoreSubsystem* ScoreSys = GI->GetSubsystem<UScoreSubsystem>())
		{
			ScoreSys->AddCoinCollected();
		}
	}

	// TODO: Play collection sound and particle burst

	// Hide and disable, then destroy after brief delay
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetLifeSpan(1.0f);
}
