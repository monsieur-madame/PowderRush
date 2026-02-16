#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PowderHUD.generated.h"

UCLASS()
class POWDERRUSH_API APowderHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
