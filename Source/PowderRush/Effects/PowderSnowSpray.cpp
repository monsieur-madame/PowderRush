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

void UPowderSnowSpray::ActivateSpray(float CarveDirection, float SprayAmount, FLinearColor SprayColor)
{
	bIsActive = true;
	if (NiagaraComp && !NiagaraComp->IsActive())
	{
		NiagaraComp->Activate();
	}
	if (NiagaraComp)
	{
		NiagaraComp->SetVariableFloat(FName("CarveDirection"), CarveDirection);
		NiagaraComp->SetVariableFloat(FName("SprayAmount"), SprayAmount);
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
