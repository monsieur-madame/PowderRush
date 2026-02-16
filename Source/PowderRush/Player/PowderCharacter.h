#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/PowderTypes.h"
#include "PowderCharacter.generated.h"

class UPowderMovementComponent;
class UCapsuleComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UPowderSnowSpray;

UCLASS()
class POWDERRUSH_API APowderCharacter : public APawn
{
	GENERATED_BODY()

public:
	APowderCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category = "PowderRush|Character")
	UPowderMovementComponent* GetPowderMovement() const { return MovementComp; }

	UFUNCTION(BlueprintPure, Category = "PowderRush|Character")
	ERiderType GetRiderType() const { return RiderType; }

	virtual UPawnMovementComponent* GetMovementComponent() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UPowderMovementComponent> MovementComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PowderRush|Character")
	TObjectPtr<UPowderSnowSpray> SnowSprayComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Character")
	ERiderType RiderType = ERiderType::Snowboarder;

	// --- Camera Tuning (Three-Quarter Diorama) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseArmLength = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float MaxArmLength = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BasePitch = -30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float SpeedPitch = -22.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float CarveYawInfluence = -5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseFOV = 72.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float MaxFOV = 82.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float ArmLengthInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float RotationInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float ReturnToFrontSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float FOVInterpSpeed = 3.0f;

	void UpdateDioramaCamera(float DeltaTime);
	void UpdateSnowSpray();
};
