#include "Terrain/PowderJump.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Effects/PowderMaterialHelper.h"
#include "Player/PowderMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

APowderJump::APowderJump()
{
	PrimaryActorTick.bCanEverTick = false;

	// Scene root — both ramp and trigger attach here so trigger isn't affected by ramp mesh scale
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// Ramp mesh: cube scaled into a thin slab, pitched upward
	RampMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RampMesh"));
	RampMesh->SetupAttachment(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeAsset.Succeeded())
	{
		RampMesh->SetStaticMesh(CubeAsset.Object);
	}

	// Scale cube into ramp slab shape (default cube is 100x100x100)
	float ScaleX = RampLength / 100.0f;
	float ScaleY = RampWidth / 100.0f;
	float ScaleZ = 0.15f; // Thin slab
	RampMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, ScaleZ));

	// Pitch upward to form a ramp — angle derived from height/length
	float RampAngle = FMath::RadiansToDegrees(FMath::Atan2(RampHeight, RampLength));
	RampMesh->SetRelativeRotation(FRotator(RampAngle, 0.0f, 0.0f));

	// Collision: terrain traces (ECC_WorldStatic) detect the ramp, but don't block player capsule
	// Must set channels manually — SetCollisionProfileName locks profile mode and ignores per-channel overrides
	RampMesh->SetCollisionObjectType(ECC_WorldStatic);
	RampMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RampMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RampMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RampMesh->ComponentTags.Add(FName(TEXT("PowderTerrain")));

	// Launch trigger: thin strip at the ramp lip (top edge) — player goes airborne when crossing it
	// Attached to SceneRoot (not RampMesh) so extents are in world-scale units
	LaunchTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LaunchTrigger"));
	LaunchTrigger->SetupAttachment(SceneRoot);
	float TriggerDepth = 60.0f; // Thin strip at the lip
	LaunchTrigger->SetBoxExtent(FVector(TriggerDepth * 0.5f, RampWidth * 0.5f, 100.0f));
	// Position at the ramp lip (top edge) in SceneRoot space
	float LipX = FMath::Cos(FMath::DegreesToRadians(RampAngle)) * RampLength * 0.5f;
	float LipZ = FMath::Sin(FMath::DegreesToRadians(RampAngle)) * RampLength * 0.5f;
	LaunchTrigger->SetRelativeLocation(FVector(LipX, 0.0f, LipZ));
	LaunchTrigger->SetRelativeRotation(FRotator(RampAngle, 0.0f, 0.0f));
	LaunchTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	LaunchTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LaunchTrigger->SetGenerateOverlapEvents(true);

	LaunchTrigger->OnComponentBeginOverlap.AddDynamic(this, &APowderJump::OnLaunchTriggerOverlap);
}

void APowderJump::BeginPlay()
{
	Super::BeginPlay();

	// Icy blue material (must be in BeginPlay — CreateColorMID asserts IsInGameThread)
	RampMesh->SetMaterial(0, PowderMaterialHelper::CreateColorMID(this, FLinearColor(0.55f, 0.78f, 0.9f)));
}

void APowderJump::OnLaunchTriggerOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// Cooldown prevents re-triggering from bounce loops
	float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastLaunchTime < LaunchCooldown)
	{
		return;
	}

	UPowderMovementComponent* MovementComp = OtherActor->FindComponentByClass<UPowderMovementComponent>();
	if (!MovementComp || MovementComp->IsAirborne())
	{
		return;
	}

	// Natural launch: transition to airborne preserving current velocity (which already has
	// upward component from riding up the ramp slope). No forced impulse — the arc comes
	// from the ramp geometry itself.
	MovementComp->LaunchIntoAir(FVector::ZeroVector);
	LastLaunchTime = Now;
}
