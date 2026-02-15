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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Character")
	ERiderType RiderType = ERiderType::Snowboarder;
};
