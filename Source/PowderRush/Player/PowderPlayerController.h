#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Core/PowderGameMode.h"
#include "PowderPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UPowderMovementComponent;
class UPowderTrickComponent;
class APowderHUD;
enum class EPowderGestureDirection : uint8;

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
	float EffectiveCarveInput = 0.0f;
	float CarveSideSwitchCooldownTimer = 0.0f;
	float CarveReleaseGraceTimer = 0.0f;
	float LastRequestedCarveSign = 0.0f;

	// Gesture detection state (airborne touch)
	FVector2D TouchStartPosition = FVector2D::ZeroVector;
	float TouchStartTime = 0.0f;
	bool bIsAirborneTouch = false;
	bool bSecondFingerActive = false;
	float BothFingersHoldTime = 0.0f;

	float SwipeThreshold = 50.0f;
	float SwipeTimeWindow = 0.3f;
	float SpreadEagleHoldThreshold = 0.4f;

	// Keyboard state tracking (desktop testing)
	bool bKeyboardCarveLeft = false;
	bool bKeyboardCarveRight = false;
	float KeyboardHoldDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Input|Feel", meta = (ClampMin = "0.0"))
	float CarveSideSwitchCooldown = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Input|Feel", meta = (ClampMin = "0.0"))
	float CarveReleaseGraceTime = 0.06f;

	UPROPERTY()
	TObjectPtr<UPowderMovementComponent> CachedMovement;

	UPROPERTY()
	TObjectPtr<UPowderTrickComponent> CachedTrickComp;

	void HandleTouchBegin(ETouchIndex::Type FingerIndex, FVector Location);
	void HandleTouchEnd(ETouchIndex::Type FingerIndex, FVector Location);
	void HandleTouchMove(ETouchIndex::Type FingerIndex, FVector Location);

	void ProcessTouchLocation(const FVector& Location);
	EPowderGestureDirection ClassifySwipe(FVector2D Delta) const;

	// Keyboard handlers for desktop testing
	void HandleKeyCarveLeftPressed();
	void HandleKeyCarveLeftReleased();
	void HandleKeyCarveRightPressed();
	void HandleKeyCarveRightReleased();

	// Keyboard trick handlers
	void HandleKeyTrickUp();
	void HandleKeyTrickDown();

	// Ollie (manual jump)
	void HandleOllie();
	float LastTapTime = 0.0f;
	float DoubleTapWindow = 0.3f;

	// Pause toggle
	void HandlePauseToggle();

	UPowderMovementComponent* GetMovementComp();
	UPowderTrickComponent* GetTrickComp();
	APowderHUD* GetPowderHUD();

	void HandleRestart();
};
