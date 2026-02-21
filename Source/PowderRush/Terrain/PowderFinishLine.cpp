#include "Terrain/PowderFinishLine.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "Core/PowderGameMode.h"
#include "Player/PowderMovementComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"

APowderFinishLine::APowderFinishLine()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(100.0f, 2500.0f, 800.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &APowderFinishLine::OnOverlapBegin);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(TriggerBox);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeAsset.Object);
	}

	// Thin red banner spanning the slope width
	VisualMesh->SetRelativeScale3D(FVector(0.1f, 50.0f, 0.3f));
}

void APowderFinishLine::BeginPlay()
{
	Super::BeginPlay();

	VisualMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.9f, 0.1f, 0.1f)));
}

void APowderFinishLine::InitExtent(float SlopeWidth)
{
	float HalfWidth = SlopeWidth * 0.5f;
	TriggerBox->SetBoxExtent(FVector(100.0f, HalfWidth, 800.0f));
	VisualMesh->SetRelativeScale3D(FVector(0.1f, SlopeWidth / 100.0f, 0.3f));
}

void APowderFinishLine::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the overlapping actor has a movement component (i.e., it's the player)
	if (!OtherActor || !OtherActor->FindComponentByClass<UPowderMovementComponent>())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (APowderGameMode* GameMode = Cast<APowderGameMode>(World->GetAuthGameMode()))
		{
			GameMode->OnFinishLineCrossed();
		}
	}
}
