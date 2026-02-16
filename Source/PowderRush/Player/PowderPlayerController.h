#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Core/PowderGameMode.h"
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

	// Keyboard state tracking (desktop testing)
	bool bKeyboardCarveLeft = false;
	bool bKeyboardCarveRight = false;
	float KeyboardHoldDuration = 0.0f;

	UPROPERTY()
	TObjectPtr<UPowderMovementComponent> CachedMovement;

	void HandleTouchBegin(ETouchIndex::Type FingerIndex, FVector Location);
	void HandleTouchEnd(ETouchIndex::Type FingerIndex, FVector Location);
	void HandleTouchMove(ETouchIndex::Type FingerIndex, FVector Location);

	void ProcessTouchLocation(const FVector& Location);

	// Keyboard handlers for desktop testing
	void HandleKeyCarveLeftPressed() { bKeyboardCarveLeft = true; KeyboardHoldDuration = 0.0f; }
	void HandleKeyCarveLeftReleased() { bKeyboardCarveLeft = false; }
	void HandleKeyCarveRightPressed() { bKeyboardCarveRight = true; KeyboardHoldDuration = 0.0f; }
	void HandleKeyCarveRightReleased() { bKeyboardCarveRight = false; }

	UPowderMovementComponent* GetMovementComp();

	void HandleRestart();
};
