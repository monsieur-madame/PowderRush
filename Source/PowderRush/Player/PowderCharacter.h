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
class UNiagaraComponent;

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
	TObjectPtr<UNiagaraComponent> SnowSprayComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Character")
	ERiderType RiderType = ERiderType::Snowboarder;

	// --- Camera Tuning (Three-Quarter Diorama) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseArmLength = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float MaxArmLength = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BasePitch = -45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float SpeedPitch = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseYawOffset = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float CarveYawInfluence = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float BaseFOV = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Camera")
	float MaxFOV = 70.0f;

	void UpdateDioramaCamera(float DeltaTime);
	void UpdateSnowSpray();
};
