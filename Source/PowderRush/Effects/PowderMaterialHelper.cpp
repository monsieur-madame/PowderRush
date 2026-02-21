#include "Effects/PowderMaterialHelper.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"

#if WITH_EDITOR
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#endif

namespace PowderMaterialHelper
{
	static TWeakObjectPtr<UMaterial> CachedMaterial;
	static TWeakObjectPtr<UMaterial> CachedSnowMaterial;
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

		// Never create dynamic materials on class defaults/archetypes.
		// Otherwise component templates can serialize private editor-only references into maps.
		if (Outer && Outer->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			return nullptr;
		}

		UObject* SafeOuter = Outer ? Outer : GetTransientPackage();

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, SafeOuter);
		if (MID)
		{
			MID->SetFlags(RF_Transient);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}

	UMaterial* GetSnowTerrainMaterial()
	{
		if (CachedSnowMaterial.IsValid())
		{
			return CachedSnowMaterial.Get();
		}

		UMaterial* Mat = LoadObject<UMaterial>(nullptr, TEXT("/Game/Materials/M_PowderSnow.M_PowderSnow"));
		if (Mat)
		{
			CachedSnowMaterial = Mat;
			return Mat;
		}

#if WITH_EDITOR
		Mat = NewObject<UMaterial>(GetTransientPackage(), TEXT("M_PowderSnow_RT"));
		Mat->SetShadingModel(MSM_DefaultLit);

		auto* ColorParam = NewObject<UMaterialExpressionVectorParameter>(Mat);
		ColorParam->ParameterName = TEXT("SnowColor");
		ColorParam->DefaultValue = FLinearColor(0.92f, 0.94f, 0.98f);

		auto* RoughnessParam = NewObject<UMaterialExpressionScalarParameter>(Mat);
		RoughnessParam->ParameterName = TEXT("Roughness");
		RoughnessParam->DefaultValue = 0.85f;

		auto* MetallicConst = NewObject<UMaterialExpressionConstant>(Mat);
		MetallicConst->R = 0.0f;

		auto& Expressions = Mat->GetExpressionCollection().Expressions;
		Expressions.Add(ColorParam);
		Expressions.Add(RoughnessParam);
		Expressions.Add(MetallicConst);

		auto* EditorData = Mat->GetEditorOnlyData();
		EditorData->BaseColor.Expression = ColorParam;
		EditorData->Roughness.Expression = RoughnessParam;
		EditorData->Metallic.Expression = MetallicConst;

		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();

		CachedSnowMaterial = Mat;
		return Mat;
#else
		return GetColorMaterial();
#endif
	}

	UMaterialInstanceDynamic* CreateSnowTerrainMID(UObject* Outer, FLinearColor SnowColor, float Roughness)
	{
		UMaterial* BaseMat = GetSnowTerrainMaterial();
		if (!BaseMat)
		{
			return nullptr;
		}

		if (Outer && Outer->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			return nullptr;
		}

		UObject* SafeOuter = Outer ? Outer : GetTransientPackage();

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, SafeOuter);
		if (MID)
		{
			MID->SetFlags(RF_Transient);
			MID->SetVectorParameterValue(TEXT("SnowColor"), SnowColor);
			MID->SetScalarParameterValue(TEXT("Roughness"), Roughness);
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

		if (Outer && Outer->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			return nullptr;
		}

		UObject* SafeOuter = Outer ? Outer : GetTransientPackage();

		UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, SafeOuter);
		if (MID)
		{
			MID->SetFlags(RF_Transient);
			MID->SetVectorParameterValue(TEXT("Color"), Color);
		}
		return MID;
	}
}
