// Copyright PowderRush. All Rights Reserved.

#include "Terrain/PowderAvalancheComponent.h"
#include "Terrain/TerrainManager.h"

UPowderAvalancheComponent::UPowderAvalancheComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPowderAvalancheComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache terrain manager from game mode
	if (APowderGameMode* GM = Cast<APowderGameMode>(GetOwner()))
	{
		CachedTerrainManager = GM->GetTerrainManager();
		GM->OnRunStateChanged.AddDynamic(this, &UPowderAvalancheComponent::OnRunStateChanged);
	}
}

void UPowderAvalancheComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive)
	{
		return;
	}

	// Lazy-cache: GameMode finds TerrainManager after component BeginPlay
	if (!CachedTerrainManager)
	{
		if (APowderGameMode* GM = Cast<APowderGameMode>(GetOwner()))
		{
			CachedTerrainManager = GM->GetTerrainManager();
		}
		if (!CachedTerrainManager)
		{
			return;
		}
	}

	// Advance timer and compute current speed
	ElapsedRunTime += DeltaTime;
	float CurrentSpeed = FMath::Clamp(BaseSpeed + AccelerationRate * ElapsedRunTime, 0.0f, MaxSpeed);

	// Advance avalanche position
	AvalancheDistance += CurrentSpeed * DeltaTime;
	AvalancheDistance = FMath::Min(AvalancheDistance, CachedTerrainManager->GetCourseLength());

	// Catch check
	float PlayerDist = CachedTerrainManager->GetPlayerDistance();
	if (PlayerDist - AvalancheDistance <= CatchDistance)
	{
		if (APowderGameMode* GM = Cast<APowderGameMode>(GetOwner()))
		{
			GM->OnWipeout();
		}
	}
}

void UPowderAvalancheComponent::ResetAvalanche()
{
	// Lazy-cache: GameMode finds TerrainManager after component BeginPlay
	if (!CachedTerrainManager)
	{
		if (APowderGameMode* GM = Cast<APowderGameMode>(GetOwner()))
		{
			CachedTerrainManager = GM->GetTerrainManager();
		}
	}

	float StartDist = CachedTerrainManager ? CachedTerrainManager->StartDistanceOffset : 0.0f;
	// Allow negative — avalanche is conceptually "behind" the course start
	AvalancheDistance = StartDist - StartOffset;
	ElapsedRunTime = 0.0f;
}

float UPowderAvalancheComponent::GetGapDistance() const
{
	if (!CachedTerrainManager)
	{
		return SafeDistance;
	}
	return CachedTerrainManager->GetPlayerDistance() - AvalancheDistance;
}

float UPowderAvalancheComponent::GetGapNormalized() const
{
	if (SafeDistance <= 0.0f)
	{
		return 1.0f;
	}
	return FMath::Clamp(GetGapDistance() / SafeDistance, 0.0f, 1.0f);
}

void UPowderAvalancheComponent::OnRunStateChanged(EPowderRunState NewState)
{
	switch (NewState)
	{
	case EPowderRunState::Starting:
		ResetAvalanche();
		bIsActive = false;
		break;

	case EPowderRunState::Running:
		bIsActive = true;
		break;

	case EPowderRunState::Paused:
	case EPowderRunState::WipedOut:
	case EPowderRunState::ScoreScreen:
	case EPowderRunState::InMenu:
		bIsActive = false;
		break;
	}
}
