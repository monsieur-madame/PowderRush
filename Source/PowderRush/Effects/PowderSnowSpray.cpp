#include "Effects/PowderSnowSpray.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"

UPowderSnowSpray::UPowderSnowSpray()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> SprayAsset(
		TEXT("/Game/Effects/NS_SnowSpray.NS_SnowSpray"));
	if (SprayAsset.Succeeded())
	{
		SpraySystem = SprayAsset.Object;
	}
}

void UPowderSnowSpray::BeginPlay()
{
	Super::BeginPlay();

	if (SpraySystem)
	{
		NiagaraComp = NewObject<UNiagaraComponent>(GetOwner());
		NiagaraComp->SetupAttachment(this);
		NiagaraComp->SetAsset(SpraySystem);
		NiagaraComp->SetAutoActivate(false);
		NiagaraComp->RegisterComponent();
	}
}

void UPowderSnowSpray::ActivateSpray(float CarveDirection)
{
	bIsActive = true;
	if (NiagaraComp && !NiagaraComp->IsActive())
	{
		NiagaraComp->Activate();
	}
	if (NiagaraComp)
	{
		NiagaraComp->SetVariableFloat(FName("CarveDirection"), CarveDirection);
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
