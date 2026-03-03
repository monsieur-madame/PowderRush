#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PowderTypes.h"
#include "PowderTrickComponent.generated.h"

class UPowderMovementComponent;
class UMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrickCompleted, EPowderTrickType, TrickType, int32, Points);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrickFailed);

UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderTrickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPowderTrickComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PowderRush|Tricks")
	bool RequestTrick(EPowderGestureDirection Gesture);

	UFUNCTION(BlueprintPure, Category = "PowderRush|Tricks")
	EPowderTrickState GetTrickState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Tricks")
	EPowderTrickType GetActiveTrickType() const { return ActiveTrickType; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Tricks")
	int32 GetTrickChainCount() const { return TrickChainCount; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Tricks")
	FPowderTrickResult GetLastJumpResult() const { return LastJumpResult; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Tricks")
	FOnTrickCompleted OnTrickCompleted;

	UPROPERTY(BlueprintAssignable, Category = "PowderRush|Tricks")
	FOnTrickFailed OnTrickFailed;

	// Trick registry — editable in Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Tricks")
	TArray<FPowderTrickDefinition> TrickDefinitions;

	// Called by movement component delegates
	UFUNCTION()
	void OnBecameAirborne();

	UFUNCTION()
	void OnLanded(float AirTime, float LandingQuality);

protected:
	EPowderTrickState CurrentState = EPowderTrickState::Idle;
	EPowderTrickType ActiveTrickType = EPowderTrickType::None;

	float TrickTimer = 0.0f;
	float TrickDuration = 0.0f;
	int32 TrickChainCount = 0;
	float ChainMultiplier = 1.0f;

	TArray<EPowderTrickType> CurrentJumpTricks;
	int32 TotalJumpPoints = 0;

	FRotator TrickStartRotation = FRotator::ZeroRotator;
	FRotator TrickTargetRotation = FRotator::ZeroRotator;

	FPowderTrickResult LastJumpResult;

	UPROPERTY()
	TObjectPtr<UPowderMovementComponent> CachedMovement;

	UPROPERTY()
	TObjectPtr<UMeshComponent> CachedBodyMesh;

	const FPowderTrickDefinition* FindTrickForGesture(EPowderGestureDirection Gesture) const;
	void CompleteTrick();
	void FailTrick();
	void ResetJumpState();
	void RestoreBaseRotation();
};
