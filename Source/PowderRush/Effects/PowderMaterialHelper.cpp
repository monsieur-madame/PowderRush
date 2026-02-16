#include "Effects/PowderMaterialHelper.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace PowderMaterialHelper
{
	static TWeakObjectPtr<UMaterial> CachedMaterial;
	static TWeakObjectPtr<UMaterial> CachedSkyMaterial;

	UMaterial* GetColorMaterial()
	{
		if (CachedMaterial.IsValid())
		{
			return CachedMaterial.Get();
		}

		UMaterial* Mat = LoadObject<UMaterial>(nullptr, TEXT("/Game/Materials/M_PowderColor.M_PowderColor"));
		if (Mat)
		{
			CachedMaterial = Mat;
		}
		return Mat;
	}

	UMaterialInstanceDynamic* CreateColorMID(UObject* Outer, FLinearColor Color)
	{
		UMaterial* BaseMat = GetColorMaterial();
		if (!BaseMat)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}

	UMaterial* GetSkyDomeMaterial()
	{
		if (CachedSkyMaterial.IsValid())
		{
			return CachedSkyMaterial.Get();
		}

		UMaterial* Mat = LoadObject<UMaterial>(nullptr, TEXT("/Game/Materials/M_PowderSkydome.M_PowderSkydome"));
		if (Mat)
		{
			CachedSkyMaterial = Mat;
		}
		return Mat;
	}

	UMaterialInstanceDynamic* CreateSkyDomeMID(UObject* Outer, FLinearColor Color)
	{
		UMaterial* BaseMat = GetSkyDomeMaterial();
		if (!BaseMat)
		{
			return nullptr;
		}

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, Outer);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}
}
