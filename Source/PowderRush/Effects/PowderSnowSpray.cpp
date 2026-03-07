#include "Effects/PowderSnowSpray.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

UPowderSnowSpray::UPowderSnowSpray()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPowderSnowSpray::BeginPlay()
{
	Super::BeginPlay();

	// Deferred load — ConstructorHelpers triggers Niagara serialization too early on iOS
	if (!SpraySystem)
	{
		SpraySystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/NS_SnowSpray.NS_SnowSpray"));
	}

	if (SpraySystem)
	{
		NiagaraComp = NewObject<UNiagaraComponent>(GetOwner());
		NiagaraComp->SetupAttachment(this);
		NiagaraComp->SetAsset(SpraySystem);
		NiagaraComp->SetAutoActivate(false);
		NiagaraComp->RegisterComponent();
	}
}

void UPowderSnowSpray::ActivateSpray(float CarveDirection, float SpeedNormalized, float SprayAmount, FLinearColor SprayColor)
{
	bIsActive = true;
	if (NiagaraComp && !NiagaraComp->IsActive())
	{
		NiagaraComp->Activate();
	}
	if (NiagaraComp)
	{
		// Offset spray laterally to the carve edge.
		// Mesh has VisualYawOffset = -90, so mesh-local axes vs movement axes:
		//   Mesh +X = movement LEFT,  Mesh -X = movement RIGHT
		//   Mesh +Y = movement FORWARD
		// CarveDirection positive = right carve, so offset to -X.
		float CarveSign = (CarveDirection > 0.0f) ? 1.0f : (CarveDirection < 0.0f) ? -1.0f : 0.0f;
		SetRelativeLocation(FVector(-CarveSign * SprayLateralOffset, 0.0f, 0.0f));

		float SpeedScale = FMath::Clamp(SpeedNormalized, 0.0f, 1.0f);

		// Pre-compute a spray velocity direction in mesh-local space for Niagara:
		// Spray kicks outward from the carving edge, slightly backward and upward.
		FVector SprayDir = FVector(-CarveSign, -0.3f, 0.4f).GetSafeNormal();

		NiagaraComp->SetVariableFloat(FName("CarveDirection"), CarveDirection);
		NiagaraComp->SetVariableFloat(FName("SprayAmount"), SprayAmount * SpeedScale);
		NiagaraComp->SetVariableFloat(FName("SpeedNormalized"), SpeedScale);
		NiagaraComp->SetVariableVec3(FName("SprayDirection"), SprayDir);
		NiagaraComp->SetVariableLinearColor(FName("SprayColor"), SprayColor);
	}
}

void UPowderSnowSpray::DeactivateSpray()
{
	bIsActive = false;
	if (NiagaraComp && NiagaraComp->IsActive())
	{
		NiagaraComp->Deactivate();
	}
}
