#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "PowderPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UPowderMovementComponent;

UCLASS()
class POWDERRUSH_API APowderPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APowderPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	// Touch input handling
	virtual bool InputTouch(uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex) override;

protected:
	// Enhanced Input Assets (set in Blueprint)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PowderRush|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PowderRush|Input")
	TObjectPtr<UInputAction> CarveLeftAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PowderRush|Input")
	TObjectPtr<UInputAction> CarveRightAction;

	// Touch state tracking
	bool bTouchActive = false;
	float TouchCarveInput = 0.0f;
	float TouchHoldDuration = 0.0f;

	UPROPERTY()
	TObjectPtr<UPowderMovementComponent> CachedMovement;

	void UpdateCarveFromTouch();
};
