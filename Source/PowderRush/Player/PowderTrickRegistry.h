#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PowderTypes.h"
#include "PowderTrickRegistry.generated.h"

/**
 * Data asset containing all trick definitions.
 * Replaces hardcoded TrickDefinitions array in UPowderTrickComponent for fully data-driven tricks.
 */
UCLASS(BlueprintType)
class POWDERRUSH_API UPowderTrickRegistry : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	TArray<FPowderTrickDefinition> Tricks;

	const FPowderTrickDefinition* FindTrickForGesture(EPowderGestureDirection Gesture) const
	{
		for (const FPowderTrickDefinition& Def : Tricks)
		{
			if (Def.RequiredGesture == Gesture)
			{
				return &Def;
			}
		}
		return nullptr;
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("PowderTrickRegistry", GetFName());
	}
};
