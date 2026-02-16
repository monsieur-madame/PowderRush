#include "Terrain/TerrainManager.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();
	ResetTerrain();
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if we need to spawn more zones ahead
	float LookAheadDistance = PlayerDistance + ZonesAhead * 20000.0f;
	while (TotalGeneratedDistance < LookAheadDistance)
	{
		SpawnZoneAhead();
	}

	// Check if we need to unload zones behind
	while (ActiveZones.Num() > 0)
	{
		const FActiveZone& OldestZone = ActiveZones[0];
		float BehindThreshold = PlayerDistance - ZonesBehind * 20000.0f;
		if (OldestZone.EndDistance < BehindThreshold)
		{
			UnloadZoneBehind();
		}
		else
		{
			break;
		}
	}
}

void ATerrainManager::ResetTerrain()
{
	// Unload all active zones
	for (auto& Zone : ActiveZones)
	{
		if (Zone.StreamingLevel)
		{
			Zone.StreamingLevel->SetShouldBeLoaded(false);
			Zone.StreamingLevel->SetShouldBeVisible(false);
		}
	}
	ActiveZones.Empty();

	PlayerDistance = 0.0f;
	TotalGeneratedDistance = 0.0f;
	ZonesGenerated = 0;

	// Pre-spawn initial zones
	for (int32 i = 0; i < ZonesAhead + 1; i++)
	{
		SpawnZoneAhead();
	}
}

void ATerrainManager::SetPlayerDistance(float Distance)
{
	PlayerDistance = Distance;
}

EZoneType ATerrainManager::GetCurrentZoneType() const
{
	for (const auto& Zone : ActiveZones)
	{
		if (PlayerDistance >= Zone.StartDistance && PlayerDistance < Zone.EndDistance)
		{
			return Zone.ZoneType;
		}
	}
	return EZoneType::PowderBowl;
}

ESurfaceType ATerrainManager::GetSurfaceTypeAtDistance(float Distance) const
{
	for (const auto& Zone : ActiveZones)
	{
		if (Distance >= Zone.StartDistance && Distance < Zone.EndDistance)
		{
			return GetDefaultSurfaceForZone(Zone.ZoneType);
		}
	}
	return ESurfaceType::Powder;
}

EZoneType ATerrainManager::SelectNextZoneType()
{
	if (ZoneDefinitions.Num() == 0)
	{
		return EZoneType::PowderBowl;
	}

	// First zone is always PowderBowl (warm-up)
	if (ZonesGenerated == 0)
	{
		return EZoneType::PowderBowl;
	}

	// Weighted random selection based on difficulty curve
	float TotalWeight = 0.0f;
	TArray<float> Weights;
	for (const auto& Zone : ZoneDefinitions)
	{
		float Weight = GetZoneWeight(Zone);
		Weights.Add(Weight);
		TotalWeight += Weight;
	}

	float Roll = FMath::FRand() * TotalWeight;
	float Accumulated = 0.0f;
	for (int32 i = 0; i < ZoneDefinitions.Num(); i++)
	{
		Accumulated += Weights[i];
		if (Roll <= Accumulated)
		{
			return ZoneDefinitions[i].ZoneType;
		}
	}

	return ZoneDefinitions.Last().ZoneType;
}

void ATerrainManager::SpawnZoneAhead()
{
	EZoneType NextType = SelectNextZoneType();

	// Find matching zone definition
	const FZoneDefinition* SelectedDef = nullptr;
	int32 SelectedIndex = -1;
	for (int32 i = 0; i < ZoneDefinitions.Num(); i++)
	{
		if (ZoneDefinitions[i].ZoneType == NextType)
		{
			SelectedDef = &ZoneDefinitions[i];
			SelectedIndex = i;
			break;
		}
	}

	float ZoneLength = SelectedDef ? SelectedDef->ZoneLength : 20000.0f;

	FActiveZone NewZone;
	NewZone.ZoneIndex = SelectedIndex;
	NewZone.ZoneType = NextType;
	NewZone.StartDistance = TotalGeneratedDistance;
	NewZone.EndDistance = TotalGeneratedDistance + ZoneLength;
	NewZone.Offset = FVector(-TotalGeneratedDistance, 0.0f, 0.0f);

	// Stream in the level if we have one
	if (SelectedDef && !SelectedDef->LevelAsset.IsNull())
	{
		bool bSuccess = false;
		FString LevelName = SelectedDef->LevelAsset.GetLongPackageName();
		ULevelStreamingDynamic* StreamingLevel = ULevelStreamingDynamic::LoadLevelInstance(
			GetWorld(), LevelName, NewZone.Offset, FRotator::ZeroRotator, bSuccess);

		if (bSuccess && StreamingLevel)
		{
			NewZone.StreamingLevel = StreamingLevel;
		}
	}

	ActiveZones.Add(NewZone);
	TotalGeneratedDistance += ZoneLength;
	ZonesGenerated++;
}

void ATerrainManager::UnloadZoneBehind()
{
	if (ActiveZones.Num() > 0)
	{
		FActiveZone& Zone = ActiveZones[0];
		if (Zone.StreamingLevel)
		{
			Zone.StreamingLevel->SetShouldBeLoaded(false);
			Zone.StreamingLevel->SetShouldBeVisible(false);
		}
		ActiveZones.RemoveAt(0);
	}
}

float ATerrainManager::GetZoneWeight(const FZoneDefinition& Zone) const
{
	float DifficultyBias = ZonesGenerated * DifficultyEscalationRate;
	// Harder zones get more weight as run progresses
	float Weight = Zone.BaseWeight;
	if (Zone.DifficultyRating > 2.0f)
	{
		Weight += DifficultyBias;
	}
	else if (Zone.DifficultyRating < 2.0f)
	{
		Weight = FMath::Max(0.1f, Weight - DifficultyBias * 0.5f);
	}
	return FMath::Max(0.1f, Weight);
}

ESurfaceType ATerrainManager::GetDefaultSurfaceForZone(EZoneType Type) const
{
	switch (Type)
	{
	case EZoneType::PowderBowl:  return ESurfaceType::Powder;
	case EZoneType::TreeSlalom:  return ESurfaceType::Groomed;
	case EZoneType::IceSheet:    return ESurfaceType::Ice;
	case EZoneType::MogulField:  return ESurfaceType::Moguls;
	case EZoneType::CliffRun:    return ESurfaceType::Groomed;
	case EZoneType::JumpPark:    return ESurfaceType::Groomed;
	default:                     return ESurfaceType::Powder;
	}
}
