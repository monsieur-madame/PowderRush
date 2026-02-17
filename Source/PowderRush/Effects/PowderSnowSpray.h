#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PowderSnowSpray.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS(ClassGroup=(PowderRush), meta=(BlueprintSpawnableComponent))
class POWDERRUSH_API UPowderSnowSpray : public USceneComponent
{
	GENERATED_BODY()

public:
	UPowderSnowSpray();
	virtual void BeginPlay() override;

	void ActivateSpray(float CarveDirection);
	void DeactivateSpray();
	bool IsSprayActive() const { return bIsActive; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowderRush|Effects")
	TObjectPtr<UNiagaraSystem> SpraySystem;

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	bool bIsActive = false;
};
