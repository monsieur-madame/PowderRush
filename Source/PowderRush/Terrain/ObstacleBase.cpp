#include "Terrain/ObstacleBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Player/PowderCharacter.h"
#include "Player/PowderMovementComponent.h"
#include "Scoring/ScoreSubsystem.h"

AObstacleBase::AObstacleBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// Static mesh for visual
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}

	// Inner collision sphere
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComp->SetupAttachment(MeshComp);
	CollisionComp->SetSphereRadius(50.0f);
	CollisionComp->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AObstacleBase::OnHit);

	// Near-miss detection sphere (outer shell)
	NearMissComp = CreateDefaultSubobject<USphereComponent>(TEXT("NearMiss"));
	NearMissComp->SetupAttachment(MeshComp);
	NearMissComp->SetSphereRadius(NearMissRadius);
	NearMissComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	NearMissComp->SetGenerateOverlapEvents(true);
	NearMissComp->OnComponentBeginOverlap.AddDynamic(this, &AObstacleBase::OnNearMissBeginOverlap);
}

void AObstacleBase::BeginPlay()
{
	Super::BeginPlay();
	NearMissComp->SetSphereRadius(NearMissRadius);
}

void AObstacleBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APowderCharacter* Player = Cast<APowderCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// Trigger wipeout on the movement component
	if (UPowderMovementComponent* Movement = Player->GetPowderMovement())
	{
		Movement->OnWipeout.Broadcast();
	}
}

void AObstacleBase::OnNearMissBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APowderCharacter* Player = Cast<APowderCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	// Award near-miss points
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UScoreSubsystem* ScoreSys = GI->GetSubsystem<UScoreSubsystem>())
		{
			if (ObstacleType == EObstacleType::Gate)
			{
				ScoreSys->AddScore(EScoreAction::GatePass);
			}
			else
			{
				ScoreSys->AddScore(EScoreAction::NearMiss);
			}
		}
	}
}
